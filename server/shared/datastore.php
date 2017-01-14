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

include_once('../config/config.php');
include_once('utils.php');

/** Handles all database operations. */
class DataStore {

// TODO temporary, make private
public $db;

function __construct()
{
    try {
        $this->db = new PDO(Config::dsn());
    } catch (PDOException $e) {
        die('Database connection failed: ' . $e->getMessage());
    }
}

/** Verify the database schema, and fix if needed. */
public function checkSchema()
{
    $currentVersion = $this->schemaVersion();
    $schemaFile = __DIR__ . '/schema.json';
    $schemaDefs = json_decode(file_get_contents($schemaFile), true);
    $targetVersion = count($schemaDefs['schema']);

    if ($currentVersion == $targetVersion) {
        echo('Schema is up-to-date.');
        return;
    }

    if ($currentVersion > $targetVersion || $currentVersion < 0)
        die('Current schema version is invalid: ' . $currentVersion . '.');

    # apply database updates
    echo('Current schema version: ' . $currentVersion . ' should be: ' . $targetVersion);
    $this->db->beginTransaction();
    for ($i = $currentVersion; $i < $targetVersion; $i++) {
        echo("Applying update $i...");
        $this->applySchemaChange($schemaDefs['schema'][$i]);
    }
    $this->db->commit();
}

/** Returns the current schema version. */
private function schemaVersion()
{
    foreach ($this->db->query('SELECT version FROM schema_version') as $row) {
        return $row['version'];
    }
    return 0;
}

/** Applies a list of schema setup commands. */
private function applySchemaChange($schemaDef)
{
    foreach($schemaDef['sql'] as $cmd) {
        $res = $this->db->exec($cmd);
        if ($res === FALSE)
            $this->fatalDbError();
    }
    $res = $this->db->exec('UPDATE schema_version SET version = ' . $schemaDef['version']);
    if ($res === FALSE)
        $this->fatalDbError();
}

/** Prints last database error and dies. */
private function fatalDbError()
{
    print_r($this->db->errorInfo());
    die('Fatal Database Error.');
}

/** List all products. */
public function allProducts()
{
    $products = array();
    $res = $this->db->query('SELECT * FROM products');
    if ($res === FALSE)
        $this->fatalDbError();
    foreach ($res as $row)
        array_push($products, $row);
    return $products;
}

/** Add a new product. */
public function addProduct($product)
{
    // create product entry
    $res = $this->db->exec('INSERT INTO products (name) VALUES (' . $this->db->quote($product['name']) . ')');
    if ($res === FALSE)
        $this->fatalDbError();

    // create product table
    $tableName = Utils::tableNameForProduct($product['name']);
    $res = $this->db->exec('CREATE TABLE ' . $tableName . ' ('
        . 'id INTEGER PRIMARY KEY AUTOINCREMENT, '
        . 'timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, '
        . 'version VARCHAR, '
        . 'platform VARCHAR, '
        . 'startCount INTEGER, '
        . 'usageTime INTEGER'
        . ')');
    if ($res === FALSE)
        $this->fatalDbError();
}

/** Delete a product. */
public function deleteProduct($product)
{
    $res = $this->db->exec('DELETE FROM products WHERE name = ' . $this->db->quote($product));
    if ($res === FALSE)
        $this->fatalDbError();

    $res = $this->db->exec('DROP TABLE ' . Utils::tableNameForProduct($product));
    if ($res === FALSE)
        $this->fatalDbError();
}

/** Look up product by name. */
public function productByName($name)
{
    $res = $this->db->query('SELECT * FROM products WHERE name = ' . $this->db->quote($name));
    if ($res === FALSE)
        $this->fatalDbError();
    foreach ($res as $row)
        return $row;
    return NULL;
}

/** Get the schema of product $productId. */
public function productSchema($productId)
{
    $res = $this->db->query('SELECT * FROM product_schema WHERE productId = ' . intval($productId));
    if ($res === FALSE)
        $this->fatalDbError();

    $schema = array();
    foreach ($res as $row) {
        $entry = array();
        $entry['name'] = $row['name'];
        $entry['type'] = $row['type'];
        array_push($schema, $entry);
    }
    return $schema;
}

/** Update product schema to $schema. */
public function updateProductSchema($productId, $schema)
{
    $oldSchema = array();
    foreach ($this->productSchema($productId) as $o) {
        $oldSchema[$o['name']] = $o;
    }

    foreach ($schema as $entry) {
        if (array_key_exists($entry['name'], $oldSchema)) {
            // update
            $res = $this->db->exec('UPDATE product_schema SET ' .
                'type = ' . $this->db->quote($entry['type']) . ' WHERE ' .
                'productId = ' . intval($productId) . ' AND ' .
                'name = ' . $this->db->quote($entry['name'])
            );
            if ($res === FALSE)
                $this->fatalDbError();
        } else {
            // insert
            $res = $this->db->exec('INSERT INTO product_schema (productId, name, type) VALUES (' .
                intval($productId) . ', ' .
                $this->db->quote($entry['name']) . ', ' .
                $this->db->quote($entry['type']) . ')'
            );
            if ($res === FALSE)
                $this->fatalDbError();
        }

        unset($oldSchema[$entry['name']]);
    }

    // delete whatever is left
    foreach($oldSchema as $entry) {
        $res = $this->db->exec('DELETE FROM product_schema WHERE ' .
            'productId = ' . intval($productId) . ' AND ' .
            'name = ' . $this->db->quote($entry['name'])
        );
        if ($res === FALSE)
            $this->fatalDbError();
    }
}

/** All data for the give product name. */
public function rawDataForProduct($name)
{
    $tableName = Utils::tableNameForProduct($name);
    $res = $this->db->query('SELECT * FROM ' . $tableName);
    if ($res === FALSE)
        $this->fatalDbError();
    $data = array();
    foreach ($res as $row) {
        $sample = array();
        $sample['timestamp'] = $row['timestamp'];
        $sample['version'] = $row['version'];
        $sample['platform'] = $row['platform'];
        $sample['startCount'] = intval($row['startCount']);
        $sample['usageTime'] = intval($row['usageTime']);
        array_push($data, $sample);
    }
    return $data;
}

/** List all survey for a product name. */
public function surveysByProductName($product)
{
    $res = $this->db->query('SELECT surveys.* FROM surveys JOIN products ON (surveys.productId = products.id) WHERE products.name = ' . $this->db->quote($product));
    if ($res === FALSE)
        $this->fatalDbError();
    $surveys = array();
    foreach ($res as $row)
        array_push($surveys, $row);
    return $surveys;
}

/** List all active surveys for a given product. */
public function activeSurveysForProduct($product)
{
    $res = $this->db->query('SELECT * FROM surveys WHERE productId = ' . $product['id'] . ' AND active = 1');
    if ($res === FALSE)
        $this->fatalDbError();
    $surveys = array();
    foreach ($res as $row)
        array_push($surveys, $row);
    return $surveys;
}

/** Add a new survey for a product given by id. */
public function addSurvey($productId, $survey)
{
    $res = $this->db->exec('INSERT INTO surveys (productId, name, url) VALUES ('
        . $productId . ', '
        . $this->db->quote($survey['name']) . ', '
        . $this->db->quote($survey['url']) . ')');
    if ($res === FALSE)
        $this->fatalDbError();
}

/** Update an existing survey with id @p $surveyId. */
public function updateSurvey($surveyId, $surveyData)
{
    $res = $this->db->exec('UPDATE surveys SET'
        . ' name = ' . $this->db->quote($surveyData['name']) . ','
        . ' url = ' . $this->db->quote($surveyData['url']) . ','
        . ' active = ' . ($surveyData['active'] ? 1 : 0)
        . ' WHERE id = ' . $surveyId);
    if ($res === FALSE)
        $this->fatalDbError();
}

/** Delete a survey given its internal id. */
public function deleteSurvey($surveyId)
{
    $res = $this->db->exec('DELETE FROM surveys WHERE id = ' . $this->db->quote($surveyId));
    if ($res === FALSE)
        $this->fatalDbError();
}

}

?>
