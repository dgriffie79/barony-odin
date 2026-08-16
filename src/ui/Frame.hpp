//! @file Frame.hpp

#pragma once

#include "../main.hpp"
#include "../draw.hpp"
#include "Font.hpp"
#include "Widget.hpp"


class Field;
class Button;
class Field;
class Slider;

//! A container for a gui (ie, a window)
//! When a frame's size is smaller than its actual size, sliders will automatically be placed in the frame.
//! Frame objects can be populated with Field objects, Button objects, other Frame objects, and more.
class Frame : public Widget {
private:
    static int _virtualScreenX;
    static int _virtualScreenY;

public:
	Frame() = delete;
	Frame(const char* _name = "");
	Frame(Frame& parent, const char* _name = "");
	Frame(const Frame&) = delete;
	Frame(Frame&&) = delete;
	~Frame();

	Frame& operator=(const Frame&) = delete;
	Frame& operator=(Frame&&) = delete;

	static int numFindFrameCalls;

	//! border style
	enum border_style_t {
		BORDER_FLAT,
		BORDER_BEVEL_HIGH,
		BORDER_BEVEL_LOW,
		BORDER_MAX
	};

	//! list text justification
	enum justify_t {
		LEFT,
		RIGHT,
		CENTER,
		JUSTIFY_TYPE_LENGTH
	};

	//! frame image
	struct image_t {
		DynamicString name;
		DynamicString path;
		Uint32 color;
		Uint32 outlineColor;
		SDL_Rect pos;
		SDL_Rect section{0, 0, 0, 0};
		bool tiled = false;
		bool disabled = false;
		bool ontop = false;
		bool outline = false;
		bool noBlitParent = false;
	};

	struct entry_t;

	//! frame list entry
	struct entry_t {
		entry_t(Frame& _parent) : parent(_parent) {}
		Frame& parent;
		DynamicString name;
		DynamicString text;
		DynamicString tooltip;
		DynamicString image;
		Uint32 color = makeColor(255, 255, 255, 255);
		void* data = nullptr;

        bool clickable = true;
		bool pressed = false;
		bool highlighted = false;
		bool leftright_control = true;
		bool leftright_allow_nonclickable = true;
		bool updown_allow_nonclickable = true;
		bool movement_nonclickable = false;
		bool navigable = true;
		Uint32 highlightTime = 0;
		bool suicide = false;

		void (*click)(entry_t&) = nullptr;
		void (*ctrlClick)(entry_t&) = nullptr;
		void (*highlight)(entry_t&) = nullptr;
		void (*highlighting)(entry_t&) = nullptr;
		void (*selected)(entry_t&) = nullptr;
	};

	//! frame processing result structure
	struct result_t {
		bool usable;			//! true if the frame is still usable after processing elements, false otherwise
		Uint32 highlightTime;	//! current time since the last frame element was pressed
		const char* tooltip;	//! tooltip for the parent frame to display
		bool removed;			//! the frame was removed during the process
	};

	//! amount of time (ms) it takes for a highlighted object to display a tooltip
	static const Uint32 tooltipTime = 500;

	//! width/height of the slider(s) that appear when actualSize > size (in pixels)
	static const Sint32 sliderSize;

	//! virtual screen size (width)
	static constexpr const int& virtualScreenX = _virtualScreenX;

	//! virtual screen size (height)
	static constexpr const int& virtualScreenY = _virtualScreenY;

	//! init ui engine
	static void guiInit();
	static void fboInit();

	//! destroy ui engine
	static void guiDestroy();
	static void fboDestroy();

    //! resize gui
    //! @param x the width of the gui (0 = fit to aspect ratio)
    //@ @param y the height of the gui (0 = fit to aspect ratio)
    static void guiResize(int x = 0, int y = 0);

	//! stuff to do before drawing anything
	static void predraw();

	//! stuff to do after drawing everything
	static void postdraw();

	//! draws the frame and all of its subelements

	public:
	void draw() const;

	//! handle clicks and other events
	//! @return compiled results of frame processing

	public:
	result_t process();

	//! to be performed recursively on every frame after process()
	void postprocess();

	//! adds a new frame to the current frame
	//! @param name internal name of the new frame
	//! @return the newly created frame
	Frame* addFrame(const char* name = "");

	//! adds a new button to the current frame
	//! @param name internal name of the new button
	//! @return the newly created button
	Button* addButton(const char* name);

	//! adds a new field to the current frame
	//! @param name internal name of the new field
	//! @param len the length of the field in characters
	//! @return the newly created field
	Field* addField(const char* name, const int len);

	//! adds a new image object to the current frame
	//! @param pos position of the image in the frame
	//! @param color the color of the image
	//! @param image the image to draw
	//! @param name the name of the image (unique id)
	//! @return the newly created image object
	image_t* addImage(const SDL_Rect pos, const Uint32 color, const char* image, const char* name = "");

	//! adds a new entry to the frame's list
	//! @param name internal name of the new entry
	//! @param resizeFrame if true, the size of the frame will be reduced after removing the entry
	//! @return the newly created entry object
	entry_t* addEntry(const char* name, bool resizeFrame);

	//! get the mouse position relative to this frame's position
	//! @param realtime always use actual mouse position instead of pre-click position
	//! @return the x and y position of the mouse (if w or h == 0 then the frame is totally clipped and invisible)
	SDL_Rect getRelativeMousePosition(bool realtime) const;

	//! adds a new slider object to the current frame
	//! @param name the name of the slider
	//! @return the newly created slider object
	Slider* addSlider(const char* name);

	//! removes all objects and list entries from the frame
	void clear();

	//! remove all list entries from the frame
	void clearEntries();
    
    //! remove an object from the frame
    //! @name the name of the object to remove
    //! @return true if an object was removed, or false if it was not found
    bool remove(const char* name);

	//! remove an entry from the frame list
	//! @param name the name of the object to remove
	//! @param resizeFrame if true, the size of the frame will be reduced after removing the entry
	//! @return true if the entry was successfully removed, false otherwise
	bool removeEntry(const char* name, bool resizeFrame);

	enum FrameSearchType : int {
		FRAME_SEARCH_DEPTH_FIRST,
		FRAME_SEARCH_BREADTH_FIRST
	};
	static FrameSearchType findFrameDefaultSearchType;
	//! recursively searches all embedded frames for a specific frame
	//! @param name the name of the frame to find
	//! @param use depth or breadth-first search
	//! @return the frame with the given name, or nullptr if the frame could not be found
	Frame* findFrame(const char* name, const FrameSearchType frameSearchType = findFrameDefaultSearchType);

	//! find a button in this frame
	//! @param name the name of the button to find
	//! @return the button, or nullptr if it could not be found
	Button* findButton(const char* name);

	//! find a field in this frame
	//! @param name the name of the field to find
	//! @return the field, or nullptr if it could not be found
	Field* findField(const char* name);

	//! find an image in this frame
	//! @param name the name of the image to find
	//! @return the image, or nullptr if it could not be found
	image_t* findImage(const char* name);

	//! find an entry in this frame
	//! @param name the name of the entry to find
	//! @return the entry, or nullptr if it could not be found
	entry_t* findEntry(const char* name);

	//! find a slider in this frame
	//! @param name the name of the slider to find
	//! @return the slider, or nullptr if it could not be found
	Slider* findSlider(const char* name);

	//! get the frame's parent frame, if any
	//! @return the parent frame, or nullptr if there is none
	Frame* getParent();

	//! resizes the frame to accomodate all list entries
	void resizeForEntries();

	//! deselect all frame elements recursively
	void deselect();

	//! activates the frame so we can select and activate list entries
	void activate();

	//! activates the current entry selection
	void activateSelection();

	//! determines if the mouse is currently within the frame or not
	//! @return true if it is, false otherwise
	bool capturesMouse() const;

	//! determines if the mouse is currently within the frame or not - but uses X/Y not OX/OY (OX/OY remain constant when dragging)
	//! @return true if it is, false otherwise
	bool capturesMouseInRealtimeCoords() const;

	//! warps the player's mouse cursor to the center location of the frame
	void warpMouseToFrame(const int player, Uint32 flags) const;

	//! gets the physical screen-space x/y (not relative to current parent - but to the absolute root)
	SDL_Rect getAbsoluteSize() const;

	//! set the list selection to the given index
	//! @param index the index to set the list selection to
	void setSelection(int index);

	//! whether to enable/disable scrolling on this frame
	//! @param enabled whether to enable or disable scrolling
	void enableScroll(bool enabled);

	//! draw an image in the frame, clipping it within the given rectangles
	//! @param image the image to draw
	//! @param _size the size of the rectangle to clip against
	//! @param scroll the amount by which to offset the image in x/y
	void drawImage(const image_t* image, const SDL_Rect& _size, const SDL_Rect& scroll) const;

	//! scroll to the current list entry selection in the frame
	//! @param scroll_to_top if true, scroll the selection to the very top of the frame
	void scrollToSelection(bool scroll_to_top = false);

	//! adds a frame to an internal list which will match our scroll values at all times.
	//! @param name the name of the frame to sync with
	void addSyncScrollTarget(const char* name);

	//! synchronizes scrolling with sync scroll targets
	void syncScroll();

	//! puts this frame on top of all others
	void bringToTop();

	//! scroll the parent frame (if any) to be within our bounds
	void scrollParent();

	const char*						getFont() const;
	const int						getBorder() const { return border; }
	const SDL_Rect&					getSize() const { return size; }
	const SDL_Rect&					getActualSize() const { return actualSize; }
	int								getBorderStyle() const { return borderStyle; }
	DynamicArrayT<Frame*>&			getFrames() { return frames; }
	DynamicArrayT<Field*>&			getFields() { return fields; }
	DynamicArrayT<Button*>&			getButtons() { return buttons; }
	DynamicArrayT<Slider*>&			getSliders() { return sliders; }
	DynamicArrayT<entry_t*>&			getEntries() { return list; }
	DynamicArrayT<image_t*>&			getImages() { return images; }
	const bool						isDisabled() const { return disabled; }
	const bool						isHollow() const { return hollow; }
	const bool						isDropDown() const { return dropDown; }
	const bool						isScrollBarsEnabled() const { return scrollbars; }
	const bool						isAllowScrollBinds() const { return allowScrollBinds; }
	const bool						isActivated() const { return activated; }
	const SDL_Rect&					getListOffset() const { return listOffset; }
	int								getSelection() const { return selection; }
	real_t							getOpacity() const { return opacity; }
	const bool						getInheritParentFrameOpacity() const { return inheritParentFrameOpacity; }
	justify_t						getJustify() const { return justify; }
	const bool						isClickable() const { return clickable; }
	const bool                      isDontTickChildren() const { return dontTickChildren; }
	int                             getEntrySize() const { return entrySize; }
	Frame*							findParentToBlitTo();
	SDL_Surface*					getBlitSurface() const { return blitSurface; }
	TempTexture*					getBlitTexture() const { return blitTexture; }
	void							setBlitChildren(bool _doBlit);
	void							setBlitDirty(bool _bBlitDity);
	void							setBlitToParent(bool _bBlitParent);
	const bool						bIsDirtyBlit() const { return bBlitDirty; }
	const bool						isBlitToParent() const { return bBlitToParent; }
	const Uint32					getTicks() const { return ticks; }

	void	setFont(const char* _font);
	void	setBorder(const int _border);
	void	setPos(const int x, const int y);
	void	setSize(SDL_Rect _size);
	void	setBorderStyle(int _borderStyle);
	void	setHigh(bool b);
	void	setColor(const Uint32& _color);
	void    setSelectedEntryColor(const Uint32& _color);
	void    setActivatedEntryColor(const Uint32& _color);
	void	setBorderColor(const Uint32& _color);
	void    setSliderColor(const Uint32& _color);
	void	setDisabled(const bool _disabled);
	void	setHollow(const bool _hollow);
	void	setDropDown(const bool _dropDown);
	void	setScrollBarsEnabled(const bool _scrollbars);
	void	setAllowScrollBinds(const bool _allow);
	void	setListOffset(SDL_Rect _size);
	void	setInheritParentFrameOpacity(const bool _inherit);
	void	setOpacity(const real_t _opacity);
	void	setListJustify(justify_t _justify);
	void	setClickable(const bool _clickable);
	void    setDontTickChildren(const bool b);
	void    setEntrySize(int _size);
	void    setActivation(entry_t* entry);
	void    setScrollWithLeftControls(const bool b);
    void    setAccelerationX(const float x);
    void    setAccelerationY(const float y);
	void	setListMenuCancelOverride(const bool b);
	void	setAllowScrollParent(const bool b);
	void	setScrollParentOffset(const SDL_Rect& offset);

	void setActualSize(SDL_Rect _actualSize);

private:
	Uint32 ticks = 0;									//!< number of engine ticks this frame has persisted
	DynamicString font = Font::defaultFont;				//!< name of the font to use for frame entries
	int border = 2;										//!< size of the frame's border
    SDL_Rect size{0, 0, 0, 0};							//!< size and position of the frame in its parent frame
	SDL_Rect actualSize{0, 0, 0, 0};					//!< size of the frame's whole contents. when larger than size, activates sliders
	border_style_t borderStyle = BORDER_BEVEL_HIGH;		//!< border style
	Uint32 color = 0;									//!< the frame's color
	Uint32 selectedEntryColor = 0;                      //!< selected entry color
	Uint32 activatedEntryColor = 0;                     //!< activated entry color
	Uint32 borderColor = 0;								//!< the frame's border color (only used for flat border)
	Uint32 sliderColor = 0;                             //!< color used for scroll sliders
	const char* tooltip = nullptr;						//!< points to the tooltip that should be displayed by the (master) frame, or nullptr if none should be displayed
	bool hollow = false;								//!< if true, the frame doesn't have a solid background
	bool draggingHSlider = false;						//!< if true, we are dragging the horizontal slider
	bool draggingVSlider = false;						//!< if true, we are dragging the vertical slider
	int oldSliderX = 0;									//!< when you start dragging a slider, this is set
	int oldSliderY = 0;									//!< when you start dragging a slider, this is set
	bool dropDown = false;								//!< if true, the frame is destroyed when specific inputs register
	Uint32 dropDownClicked = 0;							//!< key states stored for removing drop downs
	int selection = -1;									//!< entry selection
	entry_t* activation = nullptr;                      //!< activated entry
	bool allowScrollBinds = true;						//!< if true, scroll wheel + right stick can scroll frame
	bool allowScrolling = false;						//!< must be enabled for any kind of scrolling/actualSize to work
	bool scrollbars = false;							//!< must be true for sliders to be drawn/usable
	bool activated = false;								//!< true if this frame is consuming input (to navigate list entries)
	SDL_Rect listOffset{0, 0, 0, 0};					//!< frame list offset in x, y
	real_t opacity = 100.0;								//!< opacity multiplier of elements within this frame (image/fields etc)
	bool inheritParentFrameOpacity = true;				//!< if true, uses parent frame opacity
	justify_t justify = justify_t::LEFT;				//!< frame list horizontal justification
	bool clickable = false;								//!< if true, you can activate the frame by clicking on it (used for lists)
	real_t scrollX = 0.0;								//!< scroll x
	real_t scrollY = 0.0;								//!< scroll y
	real_t scrollVelocityX = 0.0;						//!< scroll velocity x
	real_t scrollVelocityY = 0.0;						//!< scroll velocity y
	real_t scrollAccelerationX = 0.0;					//!< scroll acceleration x
	real_t scrollAccelerationY = 0.0;					//!< scroll acceleration y
	bool dontTickChildren = false;                      //!< enable to prevent children from running their tick functions
	int entrySize = 0;                                  //!< the height of every entry in the list (if 0, derived from font instead)
	bool scrollWithLeftControls = true;                 //!< if true, left stick and left d-pad can scroll the frame if no items can be selected
	bool bBlitChildrenToTexture = false;				//!< if true, subframes will blit onto blitSurface and draw from this cached surface
	bool bBlitDirty = false;							//!< if true, re-blit all subframes next draw()
	bool bBlitToParent = false;							//!< if true, find a frame with findParentToBlitTo() and blit onto it's surface
	bool bListMenuListCancelOverride = false;			//!< if true, MenuListCancel will activate a widget rather than deactivating the list (i.e back_button)
	SDL_Rect scrollParentOffset{ 0,0,0,0 };				//!< scrollParent() increase/decrease amount of scrolling for parent
	bool allowScrollParent = false;						//!< if true, scrolls parent when widget movement called


	DynamicArrayT<Frame*> frames;
	DynamicArrayT<Button*> buttons;
	DynamicArrayT<Field*> fields;
	DynamicArrayT<image_t*> images;
	DynamicArrayT<Slider*> sliders;
	DynamicArrayT<entry_t*> list;

	DynamicArrayStr syncScrollTargets;

	SDL_Surface* blitSurface = nullptr;					//!< cached surface to blit to if bBlitChildrenToTexture
	TempTexture* blitTexture = nullptr;					//!< cached texture to draw to if bBlitChildrenToTexture

	//! activate the given list entry
	//! @param entry the entry to activate

	public:
	void activateEntry(entry_t& entry);

	//! draws the frame and all of its subelements
	//! @param _size real position of the frame onscreen
	//! @param _actualSize offset into the frame space (scroll)
	//! @param selectedWidgets the currently selected widgets, if any
	void draw(SDL_Rect _size, SDL_Rect _actualSize, const DynamicArrayT<Widget*>& selectedWidgets) const;

	//! draws post elements in the frame and all of its subelements
	//! @param _size real position of the frame onscreen
	//! @param _actualSize offset into the frame space (scroll)
	//! @param selectedWidgets the currently selected widgets, if any

	public:
	void drawPost(SDL_Rect _size, SDL_Rect _actualSize,
	    const DynamicArrayT<Widget*>& selectedWidgets,
	    const DynamicArrayT<Widget*>& searchParents) const;

	//! handle clicks and other events
	//! @param _size real position of the frame onscreen
	//! @param _actualSize offset into the frame space (scroll)
	//! @param usable true if another object doesn't have the mouse's attention, false otherwise
	//! @return compiled results of frame processing
	result_t process(SDL_Rect _size, SDL_Rect actualSize, const bool usable);


	public:
	bool capturesMouseImpl(SDL_Rect& _size, SDL_Rect& _actualSize, bool realtime) const;


	public:
	SDL_Rect getRelativeMousePositionImpl(SDL_Rect& _size, SDL_Rect& _actualSize, bool realtime) const;


	public:
	void processField(const SDL_Rect& _size, Field& field, Widget*& destWidget, result_t& result);

	public:
	void processButton(const SDL_Rect& _size, Button& button, Widget*& destWidget, result_t& result);

	public:
	void processSlider(const SDL_Rect& _size, Slider& slider, Widget*& destWidget, result_t& result);
};

// root frame object
extern Frame* gui;
extern bool drawingGui;
void createTestUI();
extern float uiScale;

#ifndef EDITOR
#include "../interface/consolecommand.hpp"
extern CvarBool ui_filter;
#endif
