<?php
/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

require_once('datastore.php');
require_once('product.php');
require_once('restexception.php');
require_once('schemaentry.php');
require_once('schemaentryelement.php');

/** Represents a data sample received from a user. */
class Sample
{
    /** JSON for all data of the given product. */
    public static function dataAsJson(DataStore $db, Product $product)
    {
        $data = array();
        $sampleIdIndex = array();
        $i = 0;

        // query scalar table
        $scalarSql = 'SELECT id, timestamp ';
        foreach ($product->schema as $entry) {
            if (!$entry->isScalar())
                continue;
            foreach ($entry->elements as $elem)
                $scalarSql .= ', ' . $elem->dataColumnName();
        }
        $scalarSql .= ' FROM ' . $product->dataTableName() . ' ORDER BY timestamp ASC';
        $scalarStmt = $db->prepare($scalarSql);
        $db->execute($scalarStmt, array());
        foreach ($scalarStmt as $scalarRow) {
            $rowData['id'] = intval($scalarRow['id']);
            $rowData['timestamp'] = $scalarRow['timestamp'];
            foreach ($product->schema as $entry) {
                if (!$entry->isScalar())
                    continue;
                $entryData = null;
                foreach ($entry->elements as $elem) {
                    $value = self::valueFromDb($elem, $scalarRow[$elem->dataColumnName()]);
                    if (!is_null($value))
                        $entryData[$elem->name] = $value;
                }
                if (!is_null($entryData))
                    $rowData[$entry->name] = $entryData;
            }
            array_push($data, $rowData);
            $sampleIdIndex[$rowData['id']] = $i++;
        }

        // query each non-scalar table
        foreach ($product->schema as $entry) {
            if ($entry->isScalar())
                continue;
            $sql = 'SELECT sampleId';
            if ($entry->type == SchemaEntry::MAP_TYPE)
                $sql .= ', key';
            foreach ($entry->elements as $elem)
                $sql .= ', ' . $elem->dataColumnName();
            $sql .= ' FROM ' . $entry->dataTableName() . ' ORDER BY id ASC';
            $stmt = $db->prepare($sql);
            $db->execute($stmt, array());
            foreach ($stmt as $row) {
                $entryData = null;
                foreach ($entry->elements as $elem) {
                    $value = self::valueFromDb($elem, $row[$elem->dataColumnName()]);
                    if (!is_null($value))
                        $entryData[$elem->name] = $value;
                }
                $idx = $sampleIdIndex[$row['sampleId']];
                if (!array_key_exists($entry->name, $data[$idx]))
                    $data[$idx][$entry->name] = array();
                if ($entry->type == SchemaEntry::MAP_TYPE)
                    $data[$idx][$entry->name][$row['key']] = $entryData;
                else
                    array_push($data[$idx][$entry->name], $entryData);
            }
        }

        return json_encode($data);
    }

    /** Insert a received sample for @p product into the data store. */
    public static function insert(DataStore $db, $jsonData, Product $product)
    {
        $jsonObj = json_decode($jsonData);
        if (!is_object($jsonObj))
            throw new RESTException('Invalid sample data format.', 400);
        if (property_exists($jsonObj, 'id') || property_exists($jsonObj, 'timestamp'))
            throw new RESTException('Invalid sample data.', 400);

        $sampleId = self::insertScalar($db, $jsonObj, $product);

        foreach ($product->schema as $entry) {
            if ($entry->isScalar())
                continue;
            self::insertNonScalar($db, $jsonObj, $entry, $sampleId);
        }
    }

    /** Insert a array of samples previously exported for @p product.
     *  Unlike Sample::insert, this will preserve ids and timestamps.
     */
    public static function import(DataStore $db, $jsonData, Product $product)
    {
        $jsonArray = json_decode($jsonData);
        if (!is_array($jsonArray))
            throw new RESTException('Invalid sample data format.', 400);

        foreach ($jsonArray as $sampleObj) {
            if (property_exists($sampleObj, 'id') || !property_exists($sampleObj, 'timestamp'))
                throw new RESTException('Invalid sample data.', 400);

            $sampleId = self::insertScalar($db, $sampleObj, $product);
            foreach ($product->schema as $entry) {
                if ($entry->isScalar())
                    continue;
                self::insertNonScalar($db, $sampleObj, $entry, $sampleId);
            }
        }
    }

    /** Insert scalar data for @p product.
     *  @return The sample id for use in non-scalar data tables.
     */
    private static function insertScalar(DataStore $db, $jsonObj, Product $product)
    {
        $columns = array();
        $binds = array();
        $values = array();

        if (property_exists($jsonObj, 'timestamp')) {
            array_push($columns, 'timestamp');
            array_push($binds, ':timestamp');
            array_push($values, $jsonObj->timestamp);
        }

        foreach ($product->schema as $entry) {
            if (!$entry->isScalar())
                continue;
            if (!property_exists($jsonObj, $entry->name) || !is_object($jsonObj->{$entry->name}))
                continue;
            foreach ($entry->elements as $elem) {
                if (!property_exists($jsonObj->{$entry->name}, $elem->name))
                    continue;

                $v = $jsonObj->{$entry->name}->{$elem->name};
                if (!self::isCorrectType($elem, $v))
                    continue;

                $bind = ':' . $elem->dataColumnName();
                array_push($columns, $elem->dataColumnName());
                array_push($binds, $bind);
                $values[$bind] = $v;
            }
        }

        $sql = 'INSERT INTO ' . $product->dataTableName();
        if (count($columns) > 0) {
            $sql .= ' (' . implode(', ', $columns) . ') VALUES (';
            $sql .= implode(', ', $binds) . ')';
        } else {
            $sql .= ' DEFAULT VALUES';
        }
        $stmt = $db->prepare($sql);
        $db->execute($stmt, $values);

        return $db->pdoHandle()->lastInsertId();
    }

    /** Insert non-scalar elements in seconadary data tables. */
    private static function insertNonScalar(DataStore $db, $jsonObj, SchemaEntry $schemaEntry, $sampleId)
    {
        if (!property_exists($jsonObj, $schemaEntry->name))
            return;
        $data = $jsonObj->{$schemaEntry->name};
        switch ($schemaEntry->type) {
            case SchemaEntry::LIST_TYPE:
                if (!is_array($data))
                    return;
                break;
            case SchemaEntry::MAP_TYPE:
                if (!is_object($data))
                    return;
                break;
            default:
                Utils::httpError(500, "Unknown non-scalar schema entry type.");
        }

        $columns = array('sampleId');
        $binds = array(':sampleId');
        if ($schemaEntry->type == SchemaEntry::MAP_TYPE) {
            array_push($columns, 'key');
            array_push($binds, ':key');
        }
        foreach ($schemaEntry->elements as $elem) {
            array_push($columns, $elem->dataColumnName());
            array_push($binds, ':' . $elem->dataColumnName());
        }
        $sql = 'INSERT INTO ' . $schemaEntry->dataTableName()
             . ' (' . implode(', ', $columns) . ') VALUES ('
             . implode(', ', $binds) . ')';
        $stmt = $db->prepare($sql);

        foreach ($data as $key => $entry) {
            if (!is_object($entry))
                continue;
            $bindValues = array(':sampleId' => $sampleId);
            if ($schemaEntry->type == SchemaEntry::MAP_TYPE) {
                if (!is_string($key) || strlen($key) == 0)
                    continue;
                $bindValues[':key'] = $key;
            }
            foreach ($schemaEntry->elements as $elem) {
                $bindValues[':' . $elem->dataColumnName()] = null;
                if (!property_exists($entry, $elem->name))
                    continue;
                $v = $entry->{$elem->name};
                if (!self::isCorrectType($elem, $v))
                    continue;
                $bindValues[':' . $elem->dataColumnName()] = $v;
            }
            $db->execute($stmt, $bindValues);
        }
    }

    /** Fix database output that lost the correct type for a schema entry element. */
    private static function valueFromDb(SchemaEntryElement $elem, $dbValue)
    {
        if (is_null($dbValue))
            return null;
        switch ($elem->type) {
            case SchemaEntryElement::STRING_TYPE:
                return strval($dbValue);
            case SchemaEntryElement::INT_TYPE:
                return intval($dbValue);
            case SchemaEntryElement::NUMBER_TYPE:
                return floatval($dbValue);
            case SchemaEntryElement::BOOL_TYPE:
                return boolval($dbValue);
        }
        Utils::httpError(500, "Invalid schema entry element type.");
    }

    /** Check if the given input value @p $v matches the expected type for element @p $elem. */
    private static function isCorrectType(SchemaEntryElement $elem, $v)
    {
        switch ($elem->type) {
            case SchemaEntryElement::STRING_TYPE:
                if (!is_string($v))
                    return false;
                break;
            case SchemaEntryElement::INT_TYPE:
                if (!is_int($v))
                    return false;
                break;
            case SchemaEntryElement::NUMBER_TYPE:
                if (!is_float($v))
                    return false;
                break;
            case SchemaEntryElement::BOOL_TYPE:
                if (!is_bool($v))
                    return false;
                break;
        }
        return true;
    }
}

?>
