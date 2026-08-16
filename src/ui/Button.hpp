//! @file Button.hpp

#pragma once

#include "../main.hpp"
#include "Widget.hpp"
#include "Font.hpp"

class Frame;

static inline bool rectContainsPoint(SDL_Rect r, int x, int y) {
	return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

//! A Button lives in a Frame and can have scripted actions or a native callback.
class Button : public Widget {
public:
	Button();
	Button(Frame& _parent);
	Button(const Button&) = delete;
	Button(Button&&) = delete;
	~Button() = default;

	Button& operator=(const Button&) = delete;
	Button& operator=(Button&&) = delete;

	//! the result of the button process
	struct result_t {
		bool highlighted;				//!< was highlighted this frame
		bool pressed;					//!< was pressed this frame
		bool clicked;					//!< was activated this frame
		Uint32 highlightTime;			//!< time since button was highlighted
		const char* tooltip = nullptr;	//!< button tooltip to be displayed
	};

	//! button style
	enum style_t {
		STYLE_NORMAL,
		STYLE_TOGGLE,
		STYLE_CHECKBOX,
		STYLE_RADIO,
		STYLE_DROPDOWN,
		STYLE_MAX
	};

	//! text justification
	enum justify_t {
		TOP,
		BOTTOM,
		LEFT,
		RIGHT,
		CENTER,
		JUSTIFY_TYPE_LENGTH
	};

	//! scroll the parent frame (if any) to be within our bounds
	void scrollParent();

	//! draws the button
	//! @param _size size and position of button's parent frame
	//! @param _actualSize offset into the parent frame space (scroll)
	//! @param selectedWidgets the currently selected widgets, if any
	void draw(SDL_Rect _size, SDL_Rect _actualSize, const DynamicArrayT<Widget*>& selectedWidgets) const;

	//! draws post elements on the button
	//! @param _size size and position of button's parent frame
	//! @param _actualSize offset into the parent frame space (scroll)
	//! @param selectedWidgets the currently selected widgets, if any
	void drawPost(SDL_Rect _size, SDL_Rect _actualSize,
	    const DynamicArrayT<Widget*>& selectedWidgets,
	    const DynamicArrayT<Widget*>& searchParents) const;

	//! handles button clicks, etc.
	//! @param _size size and position of button's parent frame
	//! @param _actualSize offset into the parent frame space (scroll)
	//! @param usable true if another object doesn't have the mouse's attention, false otherwise
	//! @return resultant state of the button after processing
	result_t process(SDL_Rect _size, SDL_Rect _actualSize, const bool usable);

	//! gets the physical screen-space x/y (not relative to current parent - but to the absolute root)
	SDL_Rect getAbsoluteSize() const;

	//! activates the button
	void activate();

	const char*					getText() const;
	const char*					getFont() const;
	int							getBorder() const { return border; }
	const SDL_Rect&				getSize() const { return size; }
	int							getStyle() const { return style; }
	void						(*getCallback() const)(Button&) { return callback; }
	const int					getHJustify() const;
	const int					getVJustify() const;
	const char*					getBackground() const;
	const char*					getBackgroundHighlighted() const;
	const char*					getBackgroundActivated() const;
	SDL_Rect                    getTextOffset() const;
	Uint32						getColor() const { return color; }
	Uint32						getTextColor() const { return textColor; }
	const bool					isOntop() const { return ontop; }

	void	setBorder(int _border);
	void	setPos(int x, int y);
	void	setSize(SDL_Rect _size);
	void	setColor(const Uint32& _color);
	void	setTextColor(const Uint32& _color);
	void	setTextHighlightColor(const Uint32& _color);
	void	setBorderColor(const Uint32& _color);
	void	setHighlightColor(const Uint32& _color);
	void	setText(const char* _text);
	void	setFont(const char* _font);
	void	setIcon(const char* _icon);
	void    setIconColor(const Uint32& _color);
	void	setTooltip(const char* _tooltip);
	void	setStyle(int _style);
	void	setCallback(void (*const fn)(Button&));
	void	setBackground(const char* image);
	void	setBackgroundHighlighted(const char* image);
	void	setBackgroundActivated(const char* image);
	void	setJustify(const int _justify);
	void	setHJustify(const int _justify);
	void	setVJustify(const int _justify);
	void    setTextOffset(const SDL_Rect& offset);
	void	setOntop(const bool _ontop);
	void	setPaddingPerTextLine(int padding);
	void	setScrollParentOffset(const SDL_Rect& offset);

private:
	void (*callback)(Button&) = nullptr;			//!< native callback for clicking
	DynamicString background;							//!< background image
	DynamicString backgroundHighlighted;				//!< background image when highlighted/selected
	DynamicString backgroundActivated;				//!< background image when activated
	DynamicString text;								//!< button text, if any
	DynamicString font = Font::defaultFont;			//!< button font
	DynamicString icon;								//!< icon, if any (supersedes text content)
	DynamicString tooltip;							//!< if empty, button has no tooltip; otherwise, it does
	int border = 2;									//!< size of the button border in pixels
	SDL_Rect size{0,0,0,0};							//!< size and position of the button within its parent frame
	Uint32 color = 0;								//!< the button's color
	Uint32 iconColor = 0xffffffff;                  //!< icon color
	Uint32 highlightColor = 0;						//!< color used when the button is selected/highlighted
	Uint32 textColor = 0;							//!< text color
	Uint32 textHighlightColor = 0;					//!< text color used when the button is selected/highlighted
	Uint32 borderColor = 0;							//!< (optional) border color
	style_t style = STYLE_NORMAL;					//!< button style
	justify_t hjustify = CENTER;					//!< horizontal text justification
	justify_t vjustify = CENTER;					//!< vertical text justification
	SDL_Rect textOffset{0, 0, 0, 0};                //!< offset used by label test
	bool ontop = false;								//!< whether the button is drawn ontop of others
	int paddingPerTextLine = 0;						//!< extra padding on text lines
	SDL_Rect scrollParentOffset{ 0,0,0,0 };			//!< scrollParent() increase/decrease amount of scrolling for parent
};
