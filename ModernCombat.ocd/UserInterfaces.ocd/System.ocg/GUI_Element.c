/**
	Simple proplist with functions for GUI elements.

	Provides a (in my opinion) more comfortable interface than the basic
	GUI functions.

	@author Marky
 */


/*
	Measurement prototype for GUI elements.

	Contains percent and em unit components
	of a GUI position, accessible as integers.

	Background:
	The GUI interface saves positions as strings.
	This is sufficient for simple GUIs that you program
	manually.
	However, if you want to align something in a comfortable
	way, you have to be very careful as to how you specify your
	positions, because you can add positions as much as you
	like, but you cannot subtract properly:

	Shifting an existing "1.0em" to the right by "0.5em" results in "1.0em +0.5em".
	Shifting it back by "-0.1em" results in "1.0em +0.5em -0.1em";
	As you can see, the string gets longer and longer, and subtracting that position
	from some value is impossible (so you can never really calculate the width of an element,
	for example), because
	"-%s" will yield "-1.0em +0.5em -0.1em" for that position. 
 */
static const GUI_Dimension = new Global
{
	percent = nil,
	percent_factor = nil,
	em = nil,
	em_factor = nil,


	// --- Functions

	/*
		Gets the percent component of the dimension.
	 */
	GetPercent = func ()
	{
		return this.percent;
	},

	/*
		Gets the percent component of the dimension.
	 */
	GetPercentFactor = func ()
	{
		return this.percent_factor;
	},

	/*
		Gets the em component of the dimension.
	 */	
	GetEm = func ()
	{
		return this.em;
	},

	/*
		Gets the em component factor of the dimension.
	 */	
	GetEmFactor = func ()
	{
		return this.em_factor;
	},

	/*
		Sets the percent component of the dimension.

		@par value The percent component.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	SetPercent = func (int value)
	{
		this.percent = value;
		return this;
	},

	/*
		Sets the percent component factor of the dimension.

		@par value The percent component factor.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	SetPercentFactor = func (int value)
	{
		this.percent_factor = value;
		return this;
	},

	/*
		Sets the em component of the dimension.

		@par value The em component.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	SetEm = func (int value)
	{
		this.em = value;
		return this;
	},

	/*
		Sets the em component factor of the dimension.

		@par value The em component factor.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	SetEmFactor = func (int value)
	{
		this.em_factor = value;
		return this;
	},


	/*
		Adds to the percent component of the dimension.

		@par change The percent value to add.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	AddPercent = func (int change)
	{
		SetPercent(GetPercent() + change);
		return this;
	},


	/*
		Adds to the em component of the dimension.

		@par change The em value to add.
		@return proplist The dimension itself for
		                 further function calls.
	 */
	AddEm = func (int change)
	{
		SetEm(GetEm() + change);
		return this;
	},

	CopyOf = func (proplist other)
	{
		if (IsValidPrototype(other))
		{
			return new GUI_Dimension{}
			       ->SetPercent(other->GetPercent())
			       ->SetPercentFactor(other->GetPercentFactor())
			       ->SetEm(other->GetEm())
			       ->SetEmFactor(other->GetEmFactor());
		}
		else
		{
			FatalError("Cannot copy proplist of prototype %s", GetPrototype(other));
		}
	},

	// --- Calculation functions 
	//
	// These will return a NEW dimension

	Add = func (percent, int em)
	{
		var other = Dimension(percent, em);
		AssertValidFactor(other);
		return new GUI_Dimension{}->CopyOf(this)
			       ->AddPercent(+other->GetPercent())
			       ->AddEm(+other->GetEm());
	},

	Subtract = func (percent, int em)
	{
		var other = Dimension(percent, em);
		AssertValidFactor(other);
		return new GUI_Dimension{}->CopyOf(this)
		       ->AddPercent(-other->GetPercent())
		       ->AddEm(-other->GetEm());
	},

	Scale = func(int factor)
	{
		return new GUI_Dimension{}->CopyOf(this)
		       ->SetPercent(GetPercent() * factor)
		       ->SetEm(GetEm() * factor);
	},

	Shrink = func(int factor)
	{
		return new GUI_Dimension{}->CopyOf(this)
		       ->SetPercent(GetPercent() / factor)
		       ->SetEm(GetEm() / factor);
	},

	// --- Internal functions

	AssertValidFactor = func (proplist other)
	{
		HarmonizeFactors(this, other);
		HarmonizeFactors(other, this);

		var factorP1 = other->GetPercentFactor() ?? 10;
		var factorP2 = this->GetPercentFactor() ?? 10;
		var factorE1 = other->GetEmFactor() ?? 10;
		var factorE2 = this->GetEmFactor() ?? 10;

		var error = false;


		if (factorP1 != factorP2 || factorE1 != factorE2)
		{
			FatalError("Cannot combine dimensions of different factors: other has %d%% and %dem, this has %d%% and %dem", factorP1, factorE1, factorP2, factorE2);
		}
	},

	HarmonizeFactors = func (proplist source, proplist destination)
	{
		if (!destination->GetPercent() && source->GetPercentFactor() != nil)
		{
			destination->SetPercentFactor(source->GetPercentFactor());
		}
		if (!destination->GetEm() && source->GetEmFactor() != nil)
		{
			destination->SetEmFactor(source->GetEmFactor());
		}
	},

	IsValidPrototype = func (proplist other)
	{
		return GetPrototype(other) == GUI_Dimension;
	},

	ToString = func ()
	{
		if (GetPercent() == nil && GetEm() == nil)
		{
			return nil; // Use default value!
		}
		else
		{
			// This looks a little complicated, but it is the safest way to get a correct string
			var p = "", e = "";
			if (GetPercent() != nil) p = ToPercentString(GetPercent(), GetPercentFactor());
			if (GetEm() != nil) e = ToEmString(GetEm(), GetEmFactor());
			return Format("%s%s", p, e);
		}
	},

	// Ensures that the input already is a dimension, or is converted to one.
	Dimension = func (percent_or_dimension, int em, int percent_factor, int em_factor)
	{
		if (GetType(percent_or_dimension) == C4V_PropList)
		{
			if (IsValidPrototype(percent_or_dimension))
			{
				return percent_or_dimension;
			}
			else
			{
				FatalError("Cannot use proplist of prototype %v", GetPrototype(percent_or_dimension));
			}
		}
		else
		{
			return new GUI_Dimension{}->SetPercent(percent_or_dimension)->SetEm(em)
			                          ->SetPercentFactor(percent_factor)->SetEmFactor(em_factor);
		}
	},
};


/*
	GUI prototype for a generic element.

	You can call various functions on this layout after you have created it;
	As a general rule you can change these values around as much as you like
	and they are applied to the menu only when you call layout->Update(). 

	Note:
	Positions in the original GUI are 
 */
static const GUI_Element = new Global
{
	// --- Properties:
	GUI_ID = nil,           // int gui_id - for GuiUpdate()
	GUI_ID_Child = nil,     // int child_id - for GuiUpdate()
	GUI_TargetNr = nil,     // object target - for GuiUpdate(); Contains only the number to avoid infinite proplist
	                        // (the object may contain the menu that contains this layout that saves this object as its target)
	                        // also, if the object is saved directly it is proplist, too, and therefore the GUI engine would
	                        // try to display it as a subwindow

	GUI_Element_Name = nil, // the name of the element - the property name of the sub window in the menu
	GUI_Element_Tag = nil,  // the last selected tag - for updates
	GUI_Owner = nil,        // the owner for the element - for visibility

	GUI_Element_Position = nil,// Array that contains the positions in percent and string;
	                           // Is an array, because if it were a proplist it would count as a subwindow;
	                           // Must not be initialized in the prototype, because it would be a reference and ALL gui elements would manipulate the same data

	GUI_Parent = nil,        // Array that contains the parent element;
	                         // Is an array, because if it were a proplist it would count as a subwindow;
	                         // Must not be initialized in the prototype, because it would be a reference

	GUI_Element_KeepAsChild = nil, // Keep information for this child element after opening the menu?

	// --- Generic Functions

	/**
		Gets the name of the element in the GUI layout proplist.

		@return string The name.
	 */
	GetName = func ()
	{
		return this.GUI_Element_Name;
	},

	/**
		Gets the parent GUI_Element of this element

		@return 'nil' if the element has no parent.
	 */
	GetParent = func ()
	{
		if (this.GUI_Parent == nil)
		{
			return nil;
		}
		var parent = this.GUI_Parent[0]; // Can be the same
		if (this == parent) // Prevent infinite recursion
		{
			return nil;
		}
		else
		{
			return parent;
		}
	},

	/**
		Gets the main window.
	 */
	GetRoot = func ()
	{
		var parent = GetParent();
		if (parent)
		{
			return parent->GetRoot();
		}
		else
		{
			return this;
		}
	},

	/**
		Gets the ID / GUI_ID of the main window.
	 */
	GetRootID = func ()
	{
		return GetRoot().GUI_ID;
	},

	/**
		Gets the ID of the child window.
	 */
	GetChildID = func ()
	{
		if (this.GUI_ID_Child) // Different priority, because the parent may have a different child ID, and so on.
		{
			return this.GUI_ID_Child;
		}
		else if (GetParent())
		{
			return GetParent()->GetChildID();
		}
	},

	// --- GUI Control functions

	/**
		Open as a GUI window

		@return proplist The GUI element proplist, for calling further functions.
	 */
	Open = func (int player)
	{
		if (this.GUI_ID == nil)
		{
			this->ComposeLayout();
			this.GUI_Owner = player;
			Show();

			this.GUI_ID = GuiOpen(this);
			ClearChildElements();
		}
		return this;
	},

	/**
		Close as a GUI window

		@return proplist The GUI element proplist, for calling further functions.
	 */
	Close = func ()
	{
		if (this.GUI_ID || this.ID)
		{
			// Callback
			this->~OnClose();

			// Close the GUI
			var root_id = GetRootID();
			var child_id;
			if (this.ID && this.ID != root_id)
			{
				child_id = this.ID;
			}
			GuiClose(root_id, child_id);

			// Reset the menu in a target
			if (this.Target)
			{
				var target_menu = this.Target->~GetMenu();

				if (GetType(target_menu) == C4V_PropList)
				{
					if (root_id == target_menu.ID)
					{
						this.Target->~SetMenu();
					}
				}
			}
		}
		return this;
	},

	/**
		Add to an existing GUI window as a subwindow
		Note: Use this only on other GUI_Element windows

		@par parent Add to this menu.
		@par child_id Use this child ID is a reference when updating.
		@par element_name Use this property name for the element. By default an
		                  anonoymous name is used.
        @par keep If you open the menu with Open() the property will be removed.
                  This usually ensures that the updates are not data heavy.
                  Setting this to {@code true} is necessary only if you
                  have a standalone menu and want to keep elements with a
                  special element name for future reference.
		@return proplist The GUI element proplist, for calling further functions.
	 */
	AddTo = func (proplist parent, int child_id, string element_name, bool keep)
	{
		if (IsValidPrototype(parent))
		{
			if (GetParent())
			{
				FatalError("This element already has a parent element!");
			}
			else
			{
				this.GUI_Parent = [parent]; // Save in an array to avoid infinite proplist recursion / infinite submenus
				this.GUI_ID_Child = child_id;
				this.GUI_Element_Name = element_name ?? GetValidElementName(parent);
			    this.GUI_Owner = parent.GUI_Owner;

				// Perform a GUI update on open GUIs, otherwise just add an element
				if (parent->GetRootID())
				{
					this->Update();
				}
				else
				{
					this.GUI_Element_KeepAsChild = keep;
					this->ComposeLayout();
					parent[this.GUI_Element_Name] = this;
				}
				return this;
			}
		}
		else
		{
			FatalError("Cannot add GUI element to proplist of prototype %v", GetPrototype(parent));
		}
	},

	/**
	 Sets a tag for the whole GUI or for a child.
	 */
	SetTag = func (string tag)
	{
		this.GUI_Element_Tag = tag;
		return this;
	},

	/**
		Shows the menu if the crew is enabled, hides it if disabled.

		@par crew The crew member
		@par hide_condition An additional condition for hiding the GUI
		                    element.
	 */
	ShowForCrew = func (object crew, bool hide_condition)
	{
		if (crew && crew->GetCrewEnabled() && !hide_condition)
		{
			Show()->Update({Player = this.Player});
			return true;
		}
		else
		{
			Hide()->Update({Player = this.Player});
			return false;
		}
	},

	/**
		Makes the element visible to its owner.

		@return proplist The GUI element proplist, for calling further functions.
	 */
	Show = func ()
	{
		this.Player = this.GUI_Owner;
		return this;
	},

	/**
		Makes the element invisible to its owner.

		@return proplist The GUI element proplist, for calling further functions.
	 */
	Hide = func ()
	{
		this.Player = NO_OWNER;
		return this;
	},

	/**
		Updates the GUI with all changes to the layout that were made previously.

		@par specific Updates only a specific subset of the GUI element properties, 
		              as the {@link Global#GuiUpdate} function would.
		              Defaults to the GUI element proplist itself, so that all of
		              its properties get updated in case you call the function
		              without arguments.

		@return proplist The GUI element proplist, for calling further functions.
	 */
	Update = func (proplist specific)
	{
		this->ComposeLayout();
		var gui_id = GetRootID();
		var child_id = GetChildID();
		var name = GetName();
		// Update mode: Subwindow
		var update;
		if (name && (gui_id || child_id))
		{
			// Compose a simple update proplist
			update = {};
			update[name] = specific ?? this;

			// Chain together the parent name, e.g.:
			// { subwindow_level1 = { subwindow_level2 = { element_name = {...}}}}
			for (var parent = GetParent(); parent != nil; parent = parent->GetParent())
			{
				// Cancel if the parent is the sub window known by ID, or the main window known by ID
				if (gui_id == parent.GUI_ID || // Must be GUI_ID, because the parent does not have an ID; Must not call GetRootID()
				   (child_id != nil && child_id == parent.ID)) // Child must ask the actual ID, and be defined
				{
					break; // No need for chaining proplists anymore
				}

				var chained = {};
				chained[parent->GetName()] = update;
				update = chained;
			}
		}
		// Update mode: Main window
		else if (gui_id)
		{
			update = specific ?? this;
		}
		GuiUpdate(update, gui_id, child_id);
		GuiUpdateTag(this.GUI_Element_Tag, gui_id, child_id);
		return this;
	},

	// --- Positioning functions

	// Set* functions: These leave the other values as they are,
	//                 so that the dimensions of the element can be changed

	SetLeft = func (value, int em)
	{
		InitPosition();
		this.GUI_Element_Position[0] = Dimension(value, em);
		return this;
	},

	SetRight = func (value, int em)
	{
		InitPosition();
		this.GUI_Element_Position[2] = Dimension(value, em);
		return this;
	},

	SetTop = func (value, int em)
	{
		InitPosition();
		this.GUI_Element_Position[1] = Dimension(value, em);
		return this;
	},

	SetBottom = func (value, int em)
	{
		InitPosition();
		this.GUI_Element_Position[3] = Dimension(value, em);
		return this;
	},

	// Align* functions: These leave the dimensions of the GUI element fixed,
	//                   but move its position so that it matches the target
	//                   position

	AlignLeft = func (value, int em)
	{
		var width = GetWidth();
		var position = Dimension(value, em);
		SetLeft(position);
		SetRight(position->Add(width));
		return this;
	},

	AlignRight = func (value, int em)
	{
		if (value == nil && em == nil) // Default to rightmost position if called withouth parameters
		{
			value = 1000;
		}
		var width = GetWidth();
		var position = Dimension(value, em);
		SetLeft(position->Subtract(width));
		SetRight(position);
		return this;
	},

	AlignTop = func (value, int em)
	{
		var height = GetHeight();
		var position = Dimension(value, em);
		SetTop(position);
		SetBottom(position->Add(height));
		return this;
	},

	AlignBottom = func (value, int em)
	{
		if (value == nil && em == nil) // Default to bottommost position if called withouth parameters
		{
			value = 1000;
		}
		var height = GetHeight();
		var position = Dimension(value, em);
		SetTop(position->Subtract(height));
		SetBottom(position);
		return this;
	},

	AlignCenterH = func (value, int em)
	{
		// Get the desired position
		var offset = Dimension(value, em);
		var screen_center = Dimension(500);
		var destination = screen_center->Add(offset);

		// Get the current position, with default values if not initialized
		var left = GetLeft();
		var right = GetRight();
		left->SetPercent(left->GetPercent() ?? 0);
		right->SetPercent(right->GetPercent() ?? 1000);
		var element_center = left->Add(right)->Shrink(2);

		// Shift to destination
		return ShiftRight(destination->Subtract(element_center));
	},

	AlignCenterV = func (value, int em)
	{
		// Get the desired position
		var offset = Dimension(value, em);
		var screen_center = Dimension(500);
		var destination = screen_center->Add(offset);

		// Get the current position, with default values if not initialized
		var top = GetTop();
		var bottom = GetBottom();
		top->SetPercent(top->GetPercent() ?? 0);
		bottom->SetPercent(bottom->GetPercent() ?? 1000);
		var element_center = top->Add(bottom)->Shrink(2);

		// Shift to destination
		return ShiftBottom(destination->Subtract(element_center));
	},

	// Shift* functions: These leave the dimensions of the GUI element fixed,
	//                   but move its position by a specified amount.
	//                   Shift left/right and top/bottom are somewhat redundant,
	//                   not sure if both will be kept.

	ShiftLeft = func (value, int em)
	{
		var shift = Dimension(value, em);
		SetLeft(GetLeft()->Subtract(shift));
		SetRight(GetRight()->Subtract(shift));
		return this;
	},

	ShiftRight = func (value, int em)
	{
		var shift = Dimension(value, em);
		SetLeft(GetLeft()->Add(shift));
		SetRight(GetRight()->Add(shift));
		return this;
	},

	ShiftTop = func (value, int em)
	{
		var shift = Dimension(value, em);
		SetTop(GetTop()->Subtract(shift));
		SetBottom(GetBottom()->Subtract(shift));
		return this;
	},

	ShiftBottom = func (value, int em)
	{
		var shift = Dimension(value, em);
		SetTop(GetTop()->Add(shift));
		SetBottom(GetBottom()->Add(shift));
		return this;
	},

	// Get* function: Get borders of the GUI, as a dimension.

	GetLeft = func ()
	{
		InitPosition();
		return this.GUI_Element_Position[0];
	},

	GetRight = func ()
	{
		InitPosition();
		return this.GUI_Element_Position[2];
	},

	GetTop = func ()
	{
		InitPosition();
		return this.GUI_Element_Position[1];
	},

	GetBottom = func ()
	{
		InitPosition();
		return this.GUI_Element_Position[3];
	},

	// --- Dimension related functions

	// Set* function: Sets the size of the GUI element.
	//
	//                Manipulating a GUI element this way will always
	//                change the right and/or bottom border, while
	//                the left and/or top border stay fixed.

	// Get* function: Get size of the GUI element, as a dimension.

	SetHeight = func (dimension, em)
	{
		SetBottom(GetTop()->Add(Dimension(dimension, em)));
		return this;
	},

	SetWidth = func (dimension, int em)
	{
		SetRight(GetLeft()->Add(Dimension(dimension, em)));
		return this;
	},

	GetHeight = func ()
	{
		return GetBottom()->Subtract(GetTop());
	},

	GetWidth = func ()
	{
		return GetRight()->Subtract(GetLeft());
	},

	// --- Internals

	// Removes all child elements from a GUI element
	// Background: Internally the GUI gets all children for an update before it is opened, then these are removed for less data heavy updates
	ClearChildElements = func ()
	{
		for (var property in GetProperties(this))
		{
			// Found a GUI element? Clear it!
			if (GetType(this[property]) == C4V_PropList && this[property].GUI_Element_Name)
			{
				this[property]->~ClearChildElements();
				if (!this[property].GUI_Element_KeepAsChild)
				{
					this[property] = nil;
				}
			}
		}
	},

	// Translates the integer position information to GUI layout properties
	ComposeLayout = func ()
	{
		this.Left = GetLeft()->ToString();
		this.Right = GetRight()->ToString();
		this.Top = GetTop()->ToString();
		this.Bottom = GetBottom()->ToString();

		return { Left = this.Left, Right = this.Right, Top = this.Top, Bottom = this.Bottom };
	},

	// Ensures that the input already is a dimension, or is converted to one.
	Dimension = func (percent_or_dimension, int em)
	{
		return GUI_Dimension->Dimension(percent_or_dimension, em);
	},

	IsValidPrototype = func (proplist other)
	{
		var prototypes = GetPrototypes(other);
		return GetPrototype(other) == GetPrototype(this) || IsValueInArray(prototypes, GUI_Element);
	},

	GetValidElementName = func (proplist parent)
	{
		for (var tries = 10000; tries > 0; --tries)
		{
			var element_name = Format("gui_element_%d%d%d%d%d", Random(10), Random(10), Random(10), Random(10), Random(10));
			if (GetProperty(element_name, parent) == nil)
			{
				return element_name;
			}
		}
		FatalError("Cannot find anonymous name for new GUI sub window");
	},

	InitPosition = func ()
	{
		if (!this.GUI_Element_Position)
		{
			GUI_Element_Position = [new GUI_Dimension{}, new GUI_Dimension{}, new GUI_Dimension{}, new GUI_Dimension{}];
		}
		return this;
	},

	Self = func ()
	{
		return this;
	}
};
