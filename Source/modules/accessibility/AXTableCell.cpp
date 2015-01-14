/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "modules/accessibility/AXTableCell.h"

#include "core/rendering/RenderTableCell.h"
#include "modules/accessibility/AXObjectCacheImpl.h"


namespace blink {

using namespace HTMLNames;

AXTableCell::AXTableCell(RenderObject* renderer, AXObjectCacheImpl* axObjectCache)
    : AXRenderObject(renderer, axObjectCache)
{
}

AXTableCell::~AXTableCell()
{
}

PassRefPtr<AXTableCell> AXTableCell::create(RenderObject* renderer, AXObjectCacheImpl* axObjectCache)
{
    return adoptRef(new AXTableCell(renderer, axObjectCache));
}

bool AXTableCell::isTableHeaderCell() const
{
    return node() && node()->hasTagName(thTag);
}

bool AXTableCell::computeAccessibilityIsIgnored() const
{
    AXObjectInclusion decision = defaultObjectInclusion();
    if (decision == IncludeObject)
        return false;
    if (decision == IgnoreObject)
        return true;

    if (!isTableCell())
        return AXRenderObject::computeAccessibilityIsIgnored();

    return false;
}

AXObject* AXTableCell::parentTable() const
{
    if (!m_renderer || !m_renderer->isTableCell())
        return 0;

    // If the document no longer exists, we might not have an axObjectCache.
    if (!axObjectCache())
        return 0;

    // Do not use getOrCreate. parentTable() can be called while the render tree is being modified
    // by javascript, and creating a table element may try to access the render tree while in a bad state.
    // By using only get() implies that the AXTable must be created before AXTableCells. This should
    // always be the case when AT clients access a table.
    // https://bugs.webkit.org/show_bug.cgi?id=42652
    return axObjectCache()->get(toRenderTableCell(m_renderer)->table());
}

bool AXTableCell::isTableCell() const
{
    AXObject* parent = parentObjectUnignored();
    if (!parent || !parent->isTableRow())
        return false;

    return true;
}

static AccessibilityRole decideRoleFromSibling(Node* siblingNode)
{
    if (!siblingNode)
        return CellRole;
    if (siblingNode->hasTagName(thTag))
        return ColumnHeaderRole;
    if (siblingNode->hasTagName(tdTag))
        return RowHeaderRole;
    return CellRole;
}

AccessibilityRole AXTableCell::scanToDecideHeaderRole()
{
    if (!isTableHeaderCell())
        return CellRole;

    // Check scope attribute first.
    const AtomicString& scope = getAttribute(scopeAttr);
    if (equalIgnoringCase(scope, "row"))
        return RowHeaderRole;
    if (equalIgnoringCase(scope, "col"))
        return ColumnHeaderRole;

    // Check the previous cell and the next cell
    RenderTableCell* renderCell = toRenderTableCell(m_renderer);
    AccessibilityRole headerRole = CellRole;

    // if header is preceded by header cells then it's a column header,
    // if it is preceded by cells then it's a row header.
    if (RenderTableCell* cell = renderCell->previousCell()) {
        Node* siblingNode = cell->node();
        headerRole = decideRoleFromSibling(siblingNode);
        if (headerRole != CellRole)
            return headerRole;
    }
    // if header is followed by header cells then it's a column header,
    // if it is followed by cells then it's a row header.
    if (RenderTableCell* cell = renderCell->nextCell()) {
        Node* siblingNode = cell->node();
        headerRole = decideRoleFromSibling(siblingNode);
    }
    return headerRole;
}

AccessibilityRole AXTableCell::determineAccessibilityRole()
{
    if (!isTableCell())
        return AXRenderObject::determineAccessibilityRole();

    return scanToDecideHeaderRole();
}

void AXTableCell::rowIndexRange(pair<unsigned, unsigned>& rowRange)
{
    if (!m_renderer || !m_renderer->isTableCell())
        return;

    RenderTableCell* renderCell = toRenderTableCell(m_renderer);
    rowRange.first = renderCell->rowIndex();
    rowRange.second = renderCell->rowSpan();

    // since our table might have multiple sections, we have to offset our row appropriately
    RenderTableSection* section = renderCell->section();
    RenderTable* table = renderCell->table();
    if (!table || !section)
        return;

    RenderTableSection* tableSection = table->topSection();
    unsigned rowOffset = 0;
    while (tableSection) {
        if (tableSection == section)
            break;
        rowOffset += tableSection->numRows();
        tableSection = table->sectionBelow(tableSection, SkipEmptySections);
    }

    rowRange.first += rowOffset;
}

void AXTableCell::columnIndexRange(pair<unsigned, unsigned>& columnRange)
{
    if (!m_renderer || !m_renderer->isTableCell())
        return;

    RenderTableCell* renderCell = toRenderTableCell(m_renderer);
    columnRange.first = renderCell->col();
    columnRange.second = renderCell->colSpan();
}

AXObject* AXTableCell::titleUIElement() const
{
    // Try to find if the first cell in this row is a <th>. If it is,
    // then it can act as the title ui element. (This is only in the
    // case when the table is not appearing as an AXTable.)
    if (isTableCell() || !m_renderer || !m_renderer->isTableCell())
        return 0;

    // Table cells that are th cannot have title ui elements, since by definition
    // they are title ui elements
    Node* node = m_renderer->node();
    if (node && node->hasTagName(thTag))
        return 0;

    RenderTableCell* renderCell = toRenderTableCell(m_renderer);

    // If this cell is in the first column, there is no need to continue.
    int col = renderCell->col();
    if (!col)
        return 0;

    int row = renderCell->rowIndex();

    RenderTableSection* section = renderCell->section();
    if (!section)
        return 0;

    RenderTableCell* headerCell = section->primaryCellAt(row, 0);
    if (!headerCell || headerCell == renderCell)
        return 0;

    Node* cellElement = headerCell->node();
    if (!cellElement || !cellElement->hasTagName(thTag))
        return 0;

    return axObjectCache()->getOrCreate(headerCell);
}

} // namespace blink
