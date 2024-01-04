#pragma once
#include <Tools/BrushTool.hpp>

//===========================================

// Eraser
class EraserTool: public BrushTool
{
public:
	using BrushTool::BrushTool;

	virtual const char* getIcon() const override;
	virtual const char* getName() const override;
	virtual sf::Keyboard::Key getHotkey() const;

	virtual bool onDraw() override;
	virtual void processGUI() override;

};

//===========================================