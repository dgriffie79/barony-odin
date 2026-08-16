// Widget.cpp

#include "../main.hpp"
#include "Widget.hpp"
#include "Frame.hpp"
#include "Button.hpp"
#include "Field.hpp"
#include "Slider.hpp"
#include "Image.hpp"
#include "../player.hpp"
#include "../input.hpp"
#include "../engine/audio/sound.hpp"
#include "../mod_tools.hpp"

#include <queue>

static Widget* _selectedWidgets[MAXPLAYERS] = { nullptr };

#ifndef EDITOR
CvarBool cvar_hideGlyphs("/hideprompts", false, "hide button glyphs and prompts");
#endif

Widget::~Widget() {
	if (parent) {
		for (int64_t i = 0; i < parent->widgets.size(); ++i) {
			if (parent->widgets[i] == this) {
				parent->widgets.erase(i);
				break;
			}
		}
	}
	deselectBase();
}

void Widget::deselectBase() {
    for (int c = 0; c < MAXPLAYERS; ++c) {
        if (_selectedWidgets[c] == this) {
            _selectedWidgets[c] = nullptr;
        }
    }
	selected = false;
}

bool Widget::removeBase(const char* name) {
    for (int64_t i = 0; i < widgets.size(); ++i) {
        auto widget = widgets[i];
        if (strcmp(widget->getName(), name) == 0) {
            widget->removeSelf();
            return true;
        }
    }
    return false;
}

void Widget::removeSelf() {
    toBeDeleted = true;
    
    // also mark children deleted so they don't get processed.
    for (int64_t i = 0; i < widgets.size(); ++i) {
        auto widget = widgets[i];
        widget->removeSelf();
    }
}

void Widget::select() {
	if (selected) {
		return;
	}
	Widget* head = findHead();
	if (head && head->getType() == WIDGET_FRAME) {
		Frame* f = static_cast<Frame*>(head);
		f->deselect(); // this deselects everything in the gui
	}
	if (owner >= 0 && owner < MAXPLAYERS) {
	    _selectedWidgets[owner] = this;
	}
	selected = true;
}

void Widget::deselect() {
    switch (type) {
    case WIDGET_BUTTON:
        deselectBase();
        break;
    case WIDGET_FIELD:
        static_cast<Field*>(this)->Field::deselect();
        break;
    case WIDGET_SLIDER:
        static_cast<Slider*>(this)->Slider::deselect();
        break;
    case WIDGET_FRAME:
        static_cast<Frame*>(this)->Frame::deselect();
        break;
    }
}

void Widget::activate() {
    switch (type) {
    case WIDGET_BUTTON:
        static_cast<Button*>(this)->Button::activate();
        break;
    case WIDGET_FIELD:
        static_cast<Field*>(this)->Field::activate();
        break;
    case WIDGET_SLIDER:
        static_cast<Slider*>(this)->Slider::activate();
        break;
    case WIDGET_FRAME:
        static_cast<Frame*>(this)->Frame::activate();
        break;
    }
}

void Widget::process() {
	if (!disabled && !toBeDeleted) {
		if (tickCallback) {
			(*tickCallback)(*this);
		}
	}
}

Frame* Widget::findSearchRoot() {
	Widget* gui = findHead();
	if (gui && gui->getType() == WIDGET_FRAME) {
		if (widgetSearchParent.empty()) {
			return static_cast<Frame*>(gui);
		} else {
			auto search = gui->findWidget(widgetSearchParent.c_str(), true);
			if (search && search->getType() == WIDGET_FRAME) {
				return static_cast<Frame*>(search);
			} else {
				return static_cast<Frame*>(gui);
			}
		}
	} else {
		return nullptr;
	}
}

const Frame* Widget::findSearchRoot() const {
	const Widget* gui = findHead();
	if (gui && gui->getType() == WIDGET_FRAME) {
		if (widgetSearchParent.empty()) {
			return static_cast<const Frame*>(gui);
		} else {
			auto search = gui->findWidget(widgetSearchParent.c_str(), true);
			if (search && search->getType() == WIDGET_FRAME) {
				return static_cast<const Frame*>(search);
			} else {
				return static_cast<const Frame*>(gui);
			}
		}
	} else {
		return nullptr;
	}
}

Widget* Widget::handleInput() {
	if (selected && !inputstr) {
		Input& input = Input::inputs[owner];

		// find search root
		Frame* root = nullptr;
		Widget* head = findHead();

		// move to another widget
		for (auto& move : widgetMovements) {
			if (!move.second.empty()) {
				if (input.consumeBinaryToggle(move.first)) {
					root = root ? root : findSearchRoot();
					Widget* result = root->findWidget(move.second.c_str(), true);
					if (!result && !dontSearchAncestors) {
						result = head ? head->findWidget(move.second.c_str(), true) : nullptr;
					}
					//printlog("%s: %p", move.second.c_str(), (void*)result);
					if (result && !result->disabled && !result->invisible) {
						auto in = input.input(move.first);
#ifndef EDITOR
						if (in.type != Input::binding_t::bindtype_t::MOUSE_BUTTON &&
							in.type != Input::binding_t::bindtype_t::KEYBOARD) {
							inputs.getVirtualMouse(owner)->draw_cursor = false;
						}
						playSound(495, 64);
#endif
						result->scrollParent();
						return result;
					}
				}
			}
		}

		// move to another widget and activate it
		for (auto& action : widgetActions) {
			if (!action.second.empty()) {
				if (input.consumeBinaryToggle(action.first) && !inputstr) {
					root = root ? root : findSearchRoot();
					Widget* result = root->findWidget(action.second.c_str(), true);
					if (!result && !dontSearchAncestors) {
						result = head ? head->findWidget(action.second.c_str(), true) : nullptr;
					}
					//printlog("%s: %p", action.second.c_str(), (void*)result);
					if (result && !result->disabled) {
						auto in = input.input(action.first);
#ifndef EDITOR
						if (in.type != Input::binding_t::bindtype_t::MOUSE_BUTTON &&
							in.type != Input::binding_t::bindtype_t::KEYBOARD) {
							inputs.getVirtualMouse(owner)->draw_cursor = false;
						}
#endif
						result->activate();
						return nullptr;
					}
				}
			}
		}

		// activate current selection
		if (!(menuConfirmControlType & MENU_CONFIRM_CONTROLLER) || !(menuConfirmControlType & MENU_CONFIRM_KEYBOARD)) {
			auto binding = input.input("MenuConfirm");
			if ((binding.isBindingUsingGamepad() && (menuConfirmControlType & MENU_CONFIRM_CONTROLLER)) ||
				(binding.isBindingUsingKeyboard() && (menuConfirmControlType & MENU_CONFIRM_KEYBOARD))) {
				if (input.consumeBinaryToggle("MenuConfirm") && !disabled) {
					activate();
					return nullptr;
				}
			}
		} else {
			if (input.consumeBinaryToggle("MenuConfirm") && !disabled) {
				activate();
				return nullptr;
			}
		}
	}
	return nullptr;
}

Widget* Widget::findHead() {
    if (parent && parent->owner == owner) {
        return parent->findHead();
    } else {
        return this;
    }
}

const Widget* Widget::findHead() const {
	if (parent && parent->owner == owner) {
		return parent->findHead();
	} else {
		return this;
	}
}

Widget* Widget::findWidget(const char* name, bool recursive, Widget::SearchType searchType) {
    if (searchType == Widget::SearchType::DEPTH_FIRST) {
	    for (int64_t i = 0; i < widgets.size(); ++i) {
        auto widget = widgets[i];
			if (widget->toBeDeleted) {
				continue;
			}
		    if (widget->owner != owner) {
			    continue;
		    }
		    if (widget->name == name) {
			    return widget;
		    }
		    if (recursive) {
			    auto result = widget->findWidget(name, recursive);
			    if (result) {
				    return result;
			    }
		    }
	    }
	} else if (searchType == Widget::SearchType::BREADTH_FIRST) {
		std::queue<Widget*> q;
		auto widget = this;
		do {
			for (int64_t i = 0; i < widget->widgets.size(); ++i) {
				auto w = widget->widgets[i];
				if (w->toBeDeleted) {
					continue;
				}
		        if (w->owner != owner) {
			        continue;
		        }
				if (w->name == name) {
				    return w;
				}
				if (recursive) {
				    q.push(w);
				}
			}
			if (!q.empty()) {
			    widget = q.front();
			    q.pop();
			} else {
			    break;
			}
		} while (1);
	}
	return nullptr;
}

const Widget* Widget::findWidget(const char* name, bool recursive, Widget::SearchType searchType) const {
    if (searchType == Widget::SearchType::DEPTH_FIRST) {
	    for (int64_t i = 0; i < widgets.size(); ++i) {
        auto widget = widgets[i];
			if (widget->toBeDeleted) {
				continue;
			}
		    if (widget->name == name) {
			    return widget;
		    }
		    if (recursive) {
			    auto result = widget->findWidget(name, recursive);
			    if (result) {
				    return result;
			    }
		    }
	    }
	} else if (searchType == Widget::SearchType::BREADTH_FIRST) {
		std::queue<Widget*> q;
		auto widget = this;
		do {
			for (int64_t i = 0; i < widget->widgets.size(); ++i) {
				auto w = widget->widgets[i];
			    if (w->toBeDeleted) {
				    continue;
			    }
				if (w->name == name) {
				    return w;
				}
				if (recursive) {
				    q.push(w);
				}
			}
			if (!q.empty()) {
			    widget = q.front();
			    q.pop();
			} else {
			    break;
			}
		} while (1);
	}
	return nullptr;
}

void Widget::findSelectedWidgets(DynamicArrayT<Widget*>& outResult) {
	for (int c = 0; c < MAXPLAYERS; ++c) {
	    if (_selectedWidgets[c] && _selectedWidgets[c]->isChildOf(*this)) {
	        outResult.push_back(_selectedWidgets[c]);
        } else {
            outResult.push_back(nullptr);
        }
	}
}

void Widget::findSelectedWidgets(DynamicArrayT<Widget*>& outResult) const {
	for (int c = 0; c < MAXPLAYERS; ++c) {
	    if (_selectedWidgets[c] && _selectedWidgets[c]->isChildOf(*this)) {
	        outResult.push_back(_selectedWidgets[c]);
	    } else {
            outResult.push_back(nullptr);
        }
	}
}

Widget* Widget::findSelectedWidget(int owner) {
    DynamicArrayT<Widget*> selectedWidgets;
    findSelectedWidgets(selectedWidgets);
    for (auto widget : selectedWidgets) {
        if (widget && widget->owner == owner) {
            return widget;
        }
    }
    return nullptr;
}

bool Widget::isChildOf(const Widget& widget) const {
	if (!parent) {
		return false;
	}
	else if (parent == &widget) {
		return true;
	}
	else {
		return parent->isChildOf(widget);
	}
}

void Widget::adoptWidget(Widget& widget) {
	if (widget.parent) {
		for (int64_t i = 0; i < widget.parent->widgets.size(); ++i) {
			if (widget.parent->widgets[i] == &widget) {
				widget.parent->widgets.erase(i);
				break;
			}
		}
	}
	widget.owner = owner;
	widget.parent = this;
	widget.setOwner(this->getOwner());
	widgets.push_back(&widget);
}

void Widget::drawPost(const SDL_Rect size,
    const DynamicArrayT<Widget*>& selectedWidgets,
    const DynamicArrayT<Widget*>& searchParents) const {
	if (disabled) {
		return;
	}
	const SDL_Rect viewport{0, 0, Frame::virtualScreenX, Frame::virtualScreenY};
	const Widget* selectedWidget = nullptr;
	const Widget* searchParent = nullptr;
	for (int c = 0; c < selectedWidgets.size(); ++c) {
	    auto widget = selectedWidgets[c];
		if (widget && widget->owner == owner) {
		    searchParent = searchParents[c];
			selectedWidget = widget;
			break;
		}
	}
	if (!selectedWidget) {
		return;
	} else {
		if (!alwaysShowGlyphs && searchParent && !isChildOf(*searchParent)) {
			return;
		}
	}

	// draw selector widgets
	if (!hideSelectors && selectedWidget == this && !Mods::isLoading) {
		{
			auto image = Image::get("*images/ui/Main Menus/Selector_TL.png");
			int w = image->getWidth();
			int h = image->getHeight();
			int x = size.x + selectorOffset.x;
			int y = size.y + selectorOffset.y;
			int beatx = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? w / 2 : w / 4;
			int beaty = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? h / 2 : h / 4;
			image->draw(nullptr, SDL_Rect{x - beatx, y - beaty, w, h}, viewport);
		}
		{
			auto image = Image::get("*images/ui/Main Menus/Selector_TR.png");
			int w = image->getWidth();
			int h = image->getHeight();
			int x = size.x + size.w - w + selectorOffset.w;
			int y = size.y + selectorOffset.y;
			int beatx = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? w / 2 : w / 4;
			int beaty = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? h / 2 : h / 4;
			image->draw(nullptr, SDL_Rect{x + beatx, y - beaty, w, h}, viewport);
		}
		{
			auto image = Image::get("*images/ui/Main Menus/Selector_BL.png");
			int w = image->getWidth();
			int h = image->getHeight();
			int x = size.x + selectorOffset.x;
			int y = size.y + size.h - h + selectorOffset.h;
			int beatx = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? w / 2 : w / 4;
			int beaty = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? h / 2 : h / 4;
			image->draw(nullptr, SDL_Rect{x - beatx, y + beaty, w, h}, viewport);
		}
		{
			auto image = Image::get("*images/ui/Main Menus/Selector_BR.png");
			int w = image->getWidth();
			int h = image->getHeight();
			int x = size.x + size.w - w + selectorOffset.w;
			int y = size.y + size.h - h + selectorOffset.h;
			int beatx = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? w / 2 : w / 4;
			int beaty = (ticks % TICKS_PER_SECOND) < (TICKS_PER_SECOND / 2) ? h / 2 : h / 4;
			image->draw(nullptr, SDL_Rect{x + beatx, y + beaty, w, h}, viewport);
		}
	}

	// button prompts
#ifndef EDITOR
    if (!*cvar_hideGlyphs && !hideGlyphs && !Mods::isLoading && (inputs.hasController(owner) || !hideKeyboardGlyphs)) {
        auto& actions = selectedWidget->getWidgetActions();
        auto action = actions.begin();

        const char* actionList[] = {
            "MenuConfirm",
            "MenuCancel",
            "MenuAlt1",
            "MenuAlt2",
            "MenuStart",
            "MenuSelect",
            "MenuBack",
            "MenuPageLeft",
            "MenuPageRight",
			"MenuPageLeftAlt",
			"MenuPageRightAlt",
        };
        static const int actionListSize = sizeof(actionList) / sizeof(actionList[0]);

        // set button position
        int x = size.x + buttonsOffset.x;
        int y = size.y + buttonsOffset.y;
        if (glyphPosition == CENTERED ||
            glyphPosition == CENTERED_TOP ||
            glyphPosition == CENTERED_BOTTOM) {
            x += (size.w + buttonsOffset.w) / 2;
        }
        if (glyphPosition == CENTERED_RIGHT ||
            glyphPosition == UPPER_RIGHT ||
            glyphPosition == BOTTOM_RIGHT) {
            x += size.w + buttonsOffset.w;
        }
        if (glyphPosition == CENTERED_LEFT ||
            glyphPosition == CENTERED ||
            glyphPosition == CENTERED_RIGHT) {
            y += (size.h + buttonsOffset.h) / 2;
        }
        if (glyphPosition == CENTERED_BOTTOM ||
            glyphPosition == BOTTOM_LEFT ||
            glyphPosition == BOTTOM_RIGHT) {
            y += size.h + buttonsOffset.h;
        }

        // draw glyphs
        bool pressed = ticks % TICKS_PER_SECOND < TICKS_PER_SECOND / 2;
        Input& input = Input::inputs[owner];
        for (int c = 0; c < actionListSize; ++c) {
            if ((action = actions.find(actionList[c])) != actions.end()) {
	            if (action->second == name) {
	                auto path = input.getGlyphPathForBinding(actionList[c], pressed);
		            auto image = Image::get((std::string("*") + path).c_str());
		            int w = image->getWidth();
		            int h = image->getHeight();
		            image->draw(nullptr, SDL_Rect{x - w / 2, y - h / 2, w, h}, viewport);
		            if (glyphPosition == UPPER_RIGHT ||
		                glyphPosition == CENTERED_RIGHT ||
		                glyphPosition == BOTTOM_RIGHT) {
		                x -= w;
		            } else {
		                x += w;
		            }
	            }
            } else if (c == 0 && selectedWidget == this) {
                auto path = input.getGlyphPathForBinding(actionList[c], pressed);
	            auto image = Image::get((std::string("*") + path).c_str());
	            int w = image->getWidth();
	            int h = image->getHeight();
	            image->draw(nullptr, SDL_Rect{x - w / 2, y - h / 2, w, h}, viewport);
	            if (glyphPosition == UPPER_RIGHT ||
	                glyphPosition == CENTERED_RIGHT ||
	                glyphPosition == BOTTOM_RIGHT) {
	                x -= w;
	            } else {
	                x += w;
	            }
            }
        }
    }
#endif
}

bool Widget::remove(const char* name) {
    if (type == WIDGET_FRAME) {
        return static_cast<Frame*>(this)->Frame::remove(name);
    }
    return removeBase(name);
}

void Widget::scrollParent() {
    switch (type) {
    case WIDGET_BUTTON:
        static_cast<Button*>(this)->Button::scrollParent();
        break;
    case WIDGET_FIELD:
        static_cast<Field*>(this)->Field::scrollParent();
        break;
    case WIDGET_SLIDER:
        static_cast<Slider*>(this)->Slider::scrollParent();
        break;
    case WIDGET_FRAME:
        static_cast<Frame*>(this)->Frame::scrollParent();
        break;
    }
}

const char*		    Widget::getName() const { return name.c_str(); }

const char*         Widget::getWidgetSearchParent() const { return widgetSearchParent.c_str(); }

SDL_Rect            Widget::getButtonsOffset() const { return buttonsOffset; }

SDL_Rect            Widget::getSelectorOffset() const { return selectorOffset; }

void	Widget::setName(const char* _name) { name = _name; }

void	Widget::setPressed(bool _pressed) { reallyPressed = pressed = _pressed; }

void	Widget::setDisabled(bool _disabled) { disabled = _disabled; }

void    Widget::setInvisible(bool _invisible) { invisible = _invisible; }

void    Widget::setHideGlyphs(bool _hideGlyphs) { hideGlyphs = _hideGlyphs; }

void    Widget::setHideKeyboardGlyphs(bool _hideGlyphs) { hideKeyboardGlyphs = _hideGlyphs; }

void    Widget::setHideSelectors(bool _hideSelectors) { hideSelectors = _hideSelectors; }

void    Widget::setOwner(Sint32 _owner) { owner = _owner; }

void	Widget::setTickCallback(void (*const fn)(Widget&)) { tickCallback = fn; }

void	Widget::setDrawCallback(void (*const fn)(const Widget&, const SDL_Rect)) { drawCallback = fn; }

void    Widget::setWidgetRight(const char* s) { widgetMovements["MenuRight"] = s; widgetMovements["AltMenuRight"] = s; }

void    Widget::setWidgetDown(const char* s) { widgetMovements["MenuDown"] = s; widgetMovements["AltMenuDown"] = s; }

void    Widget::setWidgetLeft(const char* s) { widgetMovements["MenuLeft"] = s; widgetMovements["AltMenuLeft"] = s; }

void    Widget::setWidgetUp(const char* s) { widgetMovements["MenuUp"] = s; widgetMovements["AltMenuUp"] = s; }

void    Widget::setWidgetPageLeft(const char* s) { widgetActions["MenuPageLeft"] = s; }

void    Widget::setWidgetPageRight(const char* s) { widgetActions["MenuPageRight"] = s; }

void    Widget::setWidgetBack(const char* s) { widgetActions["MenuCancel"] = s; }

void    Widget::removeWidgetAction(const char* binding) { if ( widgetActions.find(binding) != widgetActions.end() ) { widgetActions.erase(binding); } }

void    Widget::setWidgetSearchParent(const char* s) { widgetSearchParent = s; }

void    Widget::addWidgetAction(const char* binding, const char* action) { widgetActions[binding] = action; }

void    Widget::addWidgetMovement(const char* binding, const char* action) { widgetMovements[binding] = action; }

void    Widget::setUserData(void* p) { userData = p; }

void    Widget::setButtonsOffset(SDL_Rect r) { buttonsOffset = r; }

void    Widget::setSelectorOffset(SDL_Rect r) { selectorOffset = r; }

void	Widget::setMenuConfirmControlType(int flags) { menuConfirmControlType = flags; }

void    Widget::setGlyphPosition(glyph_position_t p) { glyphPosition = p; }

void    Widget::setAlwaysShowGlyphs(bool b) { alwaysShowGlyphs = b; }

void    Widget::setDontSearchAncestors(bool b) { dontSearchAncestors = b; }
