/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "aggregationelement.h"
#include "product.h"
#include "util.h"

#include <QJsonArray>
#include <QJsonObject>

using namespace KUserFeedback::Console;

static const struct {
    AggregationElement::Type type;
    const char *name;
} aggregation_element_types_table[] {
    { AggregationElement::Value, "value" },
    { AggregationElement::Size, "size" }
};

AggregationElement::AggregationElement() = default;
AggregationElement::~AggregationElement() = default;

bool AggregationElement::isValid() const
{
    return !m_entry.name().isEmpty();
}

SchemaEntry AggregationElement::schemaEntry() const
{
    return m_entry;
}

void AggregationElement::setSchemaEntry(const SchemaEntry& entry)
{
    m_entry = entry;
}

SchemaEntryElement AggregationElement::schemaEntryElement() const
{
    return m_element;
}

void AggregationElement::setSchemaEntryElement(const SchemaEntryElement& element)
{
    m_element = element;
}

AggregationElement::Type AggregationElement::type() const
{
    return m_type;
}

void AggregationElement::setType(AggregationElement::Type t)
{
    m_type = t;
}

QString AggregationElement::displayString() const
{
    switch (m_type) {
        case Value:
           return m_entry.name() + QLatin1Char('.') + m_element.name();
        case Size:
            return m_entry.name() + QLatin1String("[size]");
    }
    Q_UNREACHABLE();
}

bool AggregationElement::operator==(const AggregationElement &other) const
{
    if (m_type != other.m_type)
        return false;

    switch (m_type) {
        case Value:
            return m_element.name() == other.m_element.name() && m_entry.name() == other.m_entry.name();
        case Size:
            return m_element.name() == other.m_element.name();
    }
    Q_UNREACHABLE();
}

QJsonObject AggregationElement::toJsonObject() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QLatin1String(aggregation_element_types_table[m_type].name));
    switch (m_type) {
        case Value:
            obj.insert(QStringLiteral("schemaEntry"), m_entry.name());
            obj.insert(QStringLiteral("schemaEntryElement"), m_element.name());
            break;
        case Size:
            obj.insert(QStringLiteral("schemaEntry"), m_entry.name());
            break;
    }
    return obj;
}

QVector<AggregationElement> AggregationElement::fromJson(const Product &product, const QJsonArray& a)
{
    QVector<AggregationElement> elems;
    elems.reserve(a.size());
    for (const auto &v : a) {
        if (!v.isObject())
            continue;
        const auto obj = v.toObject();

        AggregationElement e;
        e.setType(Util::stringToEnum<AggregationElement::Type>(obj.value(QLatin1String("type")).toString(), aggregation_element_types_table));
        switch (e.type()) {
            case Value:
                e.setSchemaEntry(product.schemaEntry(obj.value(QLatin1String("schemaEntry")).toString()));
                e.setSchemaEntryElement(e.schemaEntry().element(obj.value(QLatin1String("schemaEntryElement")).toString()));
                break;
            case Size:
                e.setSchemaEntry(product.schemaEntry(obj.value(QLatin1String("schemaEntry")).toString()));
                break;
        }
        elems.push_back(e);
    }
    return elems;
}
