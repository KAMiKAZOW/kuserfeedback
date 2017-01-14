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

include_once('schemaentryelement.php');

/** Represents a product schema entry. */
class SchemaEntry
{
    public $name;
    public $type;
    public $aggregationType;
    public $elements = array();

    private $entryId = -1;
    private $product = null;

    const SCALAR_TPYE = 'scalar';
    const LIST_TYPE = 'list';
    const MAP_TYPE = 'map';

    public function __construct(Product $product)
    {
        $this->product = &$product;
    }

    /** Checks if this is a schema entry. */
    public function isValid()
    {
        if ($this->name != "")
            return true;
        return false;
    }

    /** Load product schema from storage. */
    static public function loadSchema(Datastore $db, Product &$product)
    {
        $stmt = $db->prepare('SELECT
                product_schema.id, product_schema.name, product_schema.type, product_schema.aggregation, schema_elements.name, schema_elements.type
            FROM schema_elements JOIN product_schema ON (product_schema.id = schema_elements.schemaId)
            WHERE product_schema.productId = :productId
            ORDER BY product_schema.id
        ');
        $db->execute($stmt, array(':productId' => $product->id()));
        $schema = array();
        $entry = new SchemaEntry($product);
        foreach ($stmt as $row) {
            if ($entry->entryId != $row[0]) {
                if ($entry->isValid()) {
                    array_push($schema, $entry);
                    $entry = new SchemaEntry($product);
                }
                $entry->entryId = $row[0];
                $entry->name = $row[1];
                $entry->type = $row[2];
                $entry->aggregationType = $row[3];
            }
            $elem = new SchemaEntryElement($entry);
            $elem->name = $row[4];
            $elem->type = $row[5];
            array_push($entry->elements, $elem);
        }
        if ($entry->isValid())
            array_push($schema, $entry);

        return $schema;
    }

    /** Insert a new schema entry into storage. */
    public function insert(Datastore $db, $productId)
    {
        $stmt = $db->prepare('INSERT INTO
            product_schema (productId, name, type, aggregation)
            VALUES (:productId, :name, :type, :aggregation)
        ');
        $db->execute($stmt, array(
            ':productId' => $productId,
            ':name' => $this->name,
            ':type' => $this->type,
            ':aggregation' => $this->aggregationType
        ));
        $this->entryId = $db->pdoHandle()->lastInsertId();

        foreach ($this->elements as $elem)
            $elem->insert($db, $this->entryId);

        // TODO add primary data table columns for scalars -> in separate method

        // add secondary data tables for non-scalars
        if ($this->type == self::LIST_TYPE || $this->type == self::MAP_TYPE)
            $this->createDataTable($db);
    }

    /** Update this schema entry in storage. */
    public function update(Datastore $db, SchemaEntry $newEntry)
    {
        // TODO reject type changes
        $stmt = $db->prepare('
            UPDATE product_schema SET
                aggregation = :aggregation
            WHERE id = :id'
        );
        $db->execute($stmt, array(
            ':aggregation' => $newEntry->aggregationType,
            ':id' => $this->entryId
        ));

        // update elements
        $oldElements = array();
        foreach ($this->elements as $oldElem)
            $oldElements[$oldElem->name] = $oldElem;

        foreach ($newEntry->elements as $newElem) {
            if (array_key_exists($newElem->name, $oldElements)) {
                // update
                // TODO this would require type conversion of the data!?
            } else {
                // insert
                $newElem->insert($db, $this->entryId);
                // TODO create data columns/tables
            }
            unset($oldElements[$newElem->name]);
        }

        // delete whatever is left
        // TODO also remove data
        foreach($oldElements as $elem)
            $elem->delete($db, $this->entryId);
    }

    /** Delete this schema entry from storage. */
    public function delete(Datastore $db, $productId)
    {
        // TODO drop primary data table columns
        // drop secondary data tables
        if ($this->type == self::LIST_TYPE || $this->type == self::MAP_TYPE)
            $this->dropDataTable($db);

        // delete elements
        $stmt = $db->prepare('DELETE FROM schema_elements WHERE schemaId = :id');
        $db->execute($stmt, array(':id' => $this->entryId));

        // delete entry
        $stmt = $db->prepare('DELETE FROM product_schema WHERE id = :id');
        $db->execute($stmt, array(':id' => $this->entryId));
    }

    /** Convert a JSON object into an array of SchemaEntry instances. */
    static public function fromJson($jsonArray, Product &$product)
    {
        $entries = array();
        foreach ($jsonArray as $jsonObj) {
            $e = new SchemaEntry($product);
            $e->name = $jsonObj->name;
            $e->type = $jsonObj->type;
            $e->aggregationType = $jsonObj->aggregationType;
            $e->elements = SchemaEntryElement::fromJson($jsonObj->elements, $e);
            array_push($entries, $e);
        }
        return $entries;
    }

    /** Data table name for secondary data tables. */
    private function dataTableName()
    {
        return $this->product->dataTableName() . '_' . Utils::normalizeString($this->name);
    }

    /** Create secondary data tables for non-scalar types. */
    private function createDataTable(Datastore $db)
    {
        $stmt = $db->prepare('CREATE TABLE ' . $this->dataTableName(). ' ('
            . 'id INTEGER PRIMARY KEY AUTOINCREMENT, '
            . 'sampleId INTEGER REFERENCES '
            . $this->product->dataTableName() . '(id))');
        $db->execute($stmt);
    }

    /** Drop secondary data tables for non-scalar types. */
    private function dropDataTable(Datastore $db)
    {
        $stmt = $db->prepare('DROP TABLE ' . $this->dataTableName());
        $db->execute($stmt);
    }
}

?>
