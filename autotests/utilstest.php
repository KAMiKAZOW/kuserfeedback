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

require_once('../src/server/shared/utils.php');

class UtilTest extends PHPUnit\Framework\TestCase
{
    public function testIsValidIdentifier_data()
    {
        return [
            'empty' => [ '', false ],
            'number' => [ 42, false ],
            'numstring' => [ '42', false ],
            'leading number' => [ '1foo', false ],
            'alpha only' => [ 'foo', true ],
            'leading space' => [ ' foo', false ],
            'middle space' => [ 'f o o', false ],
            'trailing space' => [ 'foo ', false ],
            'valid' => [ 'foo42', true ],
            'leading underscore' => [ '_foo', true ],
            'underscore' => [ 'f_o_o', true ],
            'dots and dashes' => [ 'org.kde.unit-test', true ],
            'control' => [ "fo\no", false ]
        ];
    }

    /** @dataProvider testIsValidIdentifier_data */
    public function testIsValidIdentifier($str, $result)
    {
        $this->assertEquals($result, Utils::isValidIdentifier($str));
    }

    public function testNormalize_data()
    {
        return [
            'empty' => [ '', '' ],
            'normal' => [ 'foo', 'foo' ],
            'dot' => [ 'org.kde.foo', 'org_kde_foo' ]
        ];
    }

    /** @dataProvider testNormalize_data */
    public function testNormalize($input, $output)
    {
        $this->assertEquals($output, Utils::normalizeString($input));
    }

    public function testPrimaryKeyColumn_data()
    {
        return [
            'sqlite' => [ 'sqlite', 'id', 'id INTEGER PRIMARY KEY AUTOINCREMENT' ],
            'pgsql' => [ 'pgsql', 'id', 'id SERIAL PRIMARY KEY' ],
            'mysql' => [ 'mysql', 'id', 'id INTEGER PRIMARY KEY AUTO_INCREMENT' ]
        ];
    }

    /** @dataProvider testPrimaryKeyColumn_data */
    public function testPrimaryKeyColumn($driver, $name, $output)
    {
        $this->assertEquals($output, Utils::primaryKeyColumnDeclaration($driver, $name));
    }

    public function testSqlStringType_data()
    {
        return [
            'sqlite' => [ 'sqlite', 'VARCHAR' ],
            'mysql' => [ 'mysql', 'VARCHAR(255)' ],
            'pgsql' => [ 'pgsql', 'VARCHAR' ],
        ];
    }

    /** @dataProvider testSqlStringType_data */
    public function testSqlStringType($driver, $output)
    {
        $this->assertEquals($output, Utils::sqlStringType($driver));
    }
}
