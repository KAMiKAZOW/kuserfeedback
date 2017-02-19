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

/** Represents a product aggregation setting. */
class Aggregation
{
    public $type;
    public $elements = array();

    /** Load aggregation settings for @p $product from storage. */
    static public function aggregationsForProduct(DataStore $db, Product $product)
    {
        $sql = 'SELECT type, elements FROM aggregation WHERE productId = :productId ORDER BY id';
        $stmt = $db->prepare($sql);
        $db->execute($stmt, array(':productId' => $product->id()));

        $aggrs = array();
        foreach ($stmt as $row) {
            $a = new Aggregation;
            $a->type = strval($row['type']);
            $a->elements = json_decode(strval($row['elements']));
            array_push($aggrs, $a);
        }
        return $aggrs;
    }

    /** Update aggregation settings for @p $product to @p $aggregations. */
    static public function update(DataStore $db, Product $product, $aggregations)
    {
        Aggregation::delete($db, $product);

        $sql = 'INSERT INTO aggregation (productId, type, elements) VALUES (:productId, :type, :elements)';
        $stmt = $db->prepare($sql);
        foreach ($aggregations as $a) {
            $db->execute($stmt, array(
                ':productId' => $product->id(),
                ':type' => $a->type,
                ':elements' => json_encode($a->elements)
            ));
        }
    }

    /** Delete all aggregation settings for @p $product. */
    static public function delete(DataStore $db, Product $product)
    {
        $sql = 'DELETE FROM aggregation WHERE productId = :productId';
        $stmt = $db->prepare($sql);
        $db->execute($stmt, array(':productId' => $product->id()));
    }

    /** Convert a JSON object into an array of Aggregation instances. */
    static public function fromJson($jsonArray)
    {
        if (!is_array($jsonArray))
            throw new RESTException('Wrong aggregation list format.', 400);

        $aggrs = array();
        foreach ($jsonArray as $jsonObj) {
            if (!is_object($jsonObj))
                throw new RESTException('Wrong aggregation format.', 400);
            if (!property_exists($jsonObj, 'type'))
                throw new RESTException('Incomplete aggregation object.', 400);

            $a = new Aggregation;
            $a->type = strval($jsonObj->type);
            if (property_exists($jsonObj, 'elements'))
                $a->elements = $jsonObj->elements;
            array_push($aggrs, $a);
        }
        return $aggrs;
    }
}

?>
