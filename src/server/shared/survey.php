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

require_once('compat.php');
require_once('datastore.php');
require_once('restexception.php');

/** Represents a survey. */
class Survey
{
    public $name = '';
    public $url = '';
    public $id = -1;
    public $active = false;

    /** Returns an array of all surveys for @p productName. */
    public function surveysForProduct(Datastore $db, $productName)
    {
        $stmt = $db->prepare('
            SELECT surveys.id, surveys.name, surveys.url, surveys.active
            FROM surveys JOIN products ON (surveys.productId = products.id)
            WHERE products.name = :productName
        ');
        $stmt->bindValue(':productName', $productName, PDO::PARAM_STR);
        $db->execute($stmt);

        $surveys = array();
        foreach ($stmt as $row) {
            $s = new Survey();
            $s->id = $row['id'];
            $s->name = $row['name'];
            $s->url = $row['url'];
            $s->active = boolval($row['active']);
            array_push($surveys, $s);
        }

        return $surveys;
    }

    /** Returns an array of all active surveys for @p product. */
    public function activeSurveysForProduct(Datastore $db, Product $product)
    {
        $stmt = $db->prepare('SELECT id, name, url FROM surveys WHERE productid = :productId AND active = :active');
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $stmt->bindValue(':active', true, PDO::PARAM_BOOL);
        $db->execute($stmt);

        $surveys = array();
        foreach ($stmt as $row) {
            $s = new Survey();
            $s->id = $row['id'];
            $s->name = $row['name'];
            $s->url = $row['url'];
            $s->active = true;
            array_push($surveys, $s);
        }

        return $surveys;
    }

    /** Insert a new survey into storage for product @p product. */
    public function insert(Datastore $db, Product $product)
    {
        $stmt = $db->prepare('
            INSERT INTO surveys (productid, name, url, active)
            VALUES (:productId, :name, :url, :active)
        ');
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $stmt->bindValue(':url', $this->url, PDO::PARAM_STR);
        $stmt->bindValue(':active', $this->active, PDO::PARAM_BOOL);
        $db->execute($stmt);
        $this->id = $db->pdoHandle()->lastInsertId();
    }

    /** Update an existing survey in storage. */
    public function update(Datastore $db)
    {
        $stmt = $db->prepare('
            UPDATE surveys SET
                name = :name,
                url = :url,
                active = :active
            WHERE id = :surveyId
        ');
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $stmt->bindValue(':url', $this->url, PDO::PARAM_STR);
        $stmt->bindValue(':active', $this->active, PDO::PARAM_BOOL);
        $stmt->bindValue(':surveyId', $this->id, PDO::PARAM_INT);
        $db->execute($stmt);
    }

    /** Delete this existing survey from storage. */
    public function delete(Datastore $db)
    {
        $stmt = $db->prepare('DELETE FROM surveys WHERE id = :surveyId');
        $stmt->bindValue(':surveyId', $this->id, PDO::PARAM_INT);
        $db->execute($stmt);
    }

    /** Create one Survey instance based on JSON input and verifies it is valid. */
    public static function fromJson($jsonString)
    {
        $jsonObj = json_decode($jsonString);
        if (!property_exists($jsonObj, 'name') || !property_exists($jsonObj, 'url') || !property_exists($jsonObj, 'active'))
            throw new RESTException('Incomplete survey object.', 400);

        $s = new Survey();
        $s->name = $jsonObj->name;
        $s->url = strval($jsonObj->url);
        $s->active = boolval($jsonObj->active);
        if (property_exists($jsonObj, 'id'))
            $s->id = $jsonObj->id;

        // verify
        if (strlen($s->name) <= 0 || !is_string($s->name))
            throw new RESTException('Empty product name.', 400);
        if (!is_numeric($s->id))
            throw new RESTException('Invalid survey id.', 400);

        return $s;
    }
}

?>
