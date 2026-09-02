#include "tool_manager.hpp"

#include <cassert>

#include <imgui.h>

ToolManager::ToolManager()
    : Color(C_WHITE)
    , ActiveTool(ToolType::Pencil)
    , BrushSize(1)
    , ActiveBrushShape(BrushShape::Square)
    , PrevBrushSize(1)
    , SavedPrevBrush(false)
{
    NewTool(ToolType::Pencil, std::make_unique<tool::Pencil>());
    NewTool(ToolType::Eraser, std::make_unique<tool::Eraser>());
    NewTool(ToolType::Rectangle, std::make_unique<tool::Rectangle>());
    NewTool(ToolType::Ellipse, std::make_unique<tool::Ellipse>());
    NewTool(ToolType::Pipette, std::make_unique<tool::Pipette>());
    NewTool(ToolType::Bucket, std::make_unique<tool::Bucket>());

    CurrentBrush = CreateBrushCircle(BrushSize, Color);
}

void ToolManager::SetActive(ToolType type)
{
    ActiveTool = type;
    if (type == ToolType::Pipette || type == ToolType::Bucket)
    {
        if (!SavedPrevBrush)
        {
            PrevBrushSize = BrushSize;
            SavedPrevBrush = true;
        }
        SetBrushSize(1);
    }
    else {
        if (SavedPrevBrush)
        {
            SavedPrevBrush = false;
            SetBrushSize(PrevBrushSize);
        }
        else
        {
            SetBrushSize(BrushSize);
        }
    }
}

void ToolManager::SetBrushColor(const RGBA& color)
{
    Color = color;
    BrushUpdate();
}

ToolBase* ToolManager::GetActiveTool() const
{
    auto it = ToolMap.find(ActiveTool);
    return (it != ToolMap.end() ? it->second.get() : nullptr);
}

void ToolManager::SetBrushShape(BrushShape shape)
{
    ActiveBrushShape = shape;
    switch(shape)
    {
        case BrushShape::Square: CurrentBrush = CreateBrushSquare(BrushSize, Color); break;
        case BrushShape::Circle: CurrentBrush = CreateBrushCircle(BrushSize, Color); break;
    }
}

void ToolManager::SetBrushSize(int size)
{
    BrushSize = size;
    BrushUpdate();
}

void ToolManager::BrushUpdate()
{
    SetBrushShape(ActiveBrushShape);
}