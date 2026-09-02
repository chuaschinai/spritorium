#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>

#include "tool_base.hpp"
#include "brush.hpp"

enum class ToolType
{
    Pencil,
    Eraser,
    Rectangle,
    Ellipse,
    Pipette,
    Bucket
};

enum class BrushShape
{
    Square,
    Circle
};

inline const char* GetNameFromToolType(ToolType tool_type)
{
    switch (tool_type)
    {
        case ToolType::Pencil: return "Pencil";
        case ToolType::Eraser: return "Eraser";
        case ToolType::Rectangle: return "Rectangle";
        case ToolType::Ellipse: return "Ellipse";
        case ToolType::Pipette: return "Pipette";
        case ToolType::Bucket: return "Bucket";
    }
}

class ToolManager
{
public:
    RGBA Color;
    ToolType ActiveTool;
    std::unique_ptr<Brush> CurrentBrush;
    int BrushSize;
    BrushShape ActiveBrushShape;

    ToolManager();

    void SetActive(ToolType type);
    ToolBase* GetActiveTool() const;
    void SetBrushShape(BrushShape shape);
    void SetBrushSize(int size);
    void SetBrushColor(const RGBA& color);
    void BrushUpdate();
    
private:
    int PrevBrushSize;
    bool SavedPrevBrush;

    void NewTool(ToolType type, std::unique_ptr<ToolBase> tool) { ToolMap[type] = std::move(tool); }

    std::unordered_map<ToolType, std::unique_ptr<ToolBase>> ToolMap;
};