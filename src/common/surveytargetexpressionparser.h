/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

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

#ifndef USERFEEDBACK_SURVEYTARGETEXPRESSIONPARSER_H
#define USERFEEDBACK_SURVEYTARGETEXPRESSIONPARSER_H

class QString;

namespace UserFeedback {

class SurveyTargetExpression;

class SurveyTargetExpressionParser
{
public:
    SurveyTargetExpressionParser();
    ~SurveyTargetExpressionParser();

    bool parse(const QString &s);

    SurveyTargetExpression *expression() const;

private:
    SurveyTargetExpression *m_expression;
};

}

#endif // USERFEEDBACK_SURVEYTARGETEXPRESSIONPARSER_H
