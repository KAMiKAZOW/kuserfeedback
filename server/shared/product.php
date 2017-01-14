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

require_once('schemaentry.php');
require_once('utils.php');

/** Represents a product and its schema. */
class Product
{
    public $name;
    public $schema = array();

    private $productId = -1;

    /** Name of the primary data table for this product, ie.
     *  the one containing all scalar data.
     */
    public function dataTableName()
    {
        $tableName = 'product_' . Utils::normalizeString($this->name);
        return $tableName;
    }

    /** Returns the numeric database id of this product for use in queries. */
    public function id()
    {
        return $this->productId;
    }

    /** Retrieve all products from storage. */
    public static function allProducts(Datastore $db)
    {
        $products = array();
        $stmt = $db->prepare('SELECT * FROM products');
        $db->execute($stmt);
        foreach ($stmt as $row) {
            $p = new Product();
            $p->productId = $row['id'];
            $p->name = $row['name'];
            $p->schema = SchemaEntry::loadSchema($db, $p);
            array_push($products, $p);
        }
        return $products;
    }

    /** Retrieve a specific product by name from storage. */
    public static function productByName(Datastore $db, $name)
    {
        $stmt = $db->prepare('SELECT * FROM products WHERE name = :name');
        $db->execute($stmt, array(':name' => $name));
        foreach ($stmt as $row) {
            $p = new Product();
            $p->productId = $row['id'];
            $p->name = $row['name'];
            $p->schema = SchemaEntry::loadSchema($db, $p);
            return $p;
        }
        return null;
    }

    /** Insert new product into database. */
    public function insert(Datastore $db)
    {
        // create product entry
        $stmt = $db->prepare('INSERT INTO products (name) VALUES (:name)');
        $db->execute($stmt, array(':name' => $this->name));
        $this->productId = $db->pdoHandle()->lastInsertId();

        // create data tables;
        $stmt = $db->prepare('CREATE TABLE ' . $this->dataTableName() . ' (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP)
        ');
        $db->execute($stmt);

        // create schema entries
        foreach ($this->schema as $entry)
            $entry->insert($db, $this->productId);
    }

    /** Update an existing product in the database to match @p $newProduct. */
    public function update(Datastore $db, Product $newProduct)
    {
        $oldSchema = array();
        foreach ($this->schema as $oldEntry)
            $oldSchema[$oldEntry->name] = $oldEntry;

        foreach ($newProduct->schema as $newEntry) {
            if (array_key_exists($newEntry->name, $oldSchema)) {
                // update
                $oldEntry = $oldSchema[$newEntry->name];
                $oldEntry->update($db, $newEntry);
            } else {
                // insert
                $newEntry->insert($db, $this->productId);
            }
            unset($oldSchema[$newEntry->name]);
        }

        // delete whatever is left
        foreach($oldSchema as $entry)
            $entry->delete($db, $this->productId);
    }

    /** Delete an existing product in the database. */
    public function delete(Datastore $db)
    {
        // delete schema entries
        foreach ($this->schema as $entry)
            $entry->delete($db, $this->productId);

        // delete data tables
        $stmt = $db->prepare('DROP TABLE ' . $this->dataTableName());
        $db->execute($stmt);

        // delete product
        $stmt = $db->prepare('DELETE FROM products WHERE id = :id');
        $db->execute($stmt, array(':id' => $this->productId));
    }

    /** Create one Product instance based on JSON input and verifies it is valid. */
    public static function fromJson($jsonString)
    {
        $jsonObj = json_decode($jsonString);
        if (!property_exists($jsonObj, 'name'))
            throw new RESTException('No product name specified.', 400);

        $p = new Product();
        $p->name = $jsonObj->name;
        $p->schema = SchemaEntry::fromJson($jsonObj->schema, $p);

        // verify
        if (strlen($p->name) <= 0 || !is_string($p->name))
            throw new RESTException('Empty product name.', 400);
        if (!ctype_alpha($p->name[0]))
            throw new RESTException("Invalid product name, must start with a letter.", 400);
        // TODO verify schema

        return $p;
    }

    /** Serialize product to JSON for sending to the client. */
    public function toJson()
    {
        return json_encode($this);
    }
}

?>
