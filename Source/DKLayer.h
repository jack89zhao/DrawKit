/**
 @author Contributions from the community; see CONTRIBUTORS.md
 @date 2005-2016
 @copyright MPL2; see LICENSE.txt
*/

#import <Cocoa/Cocoa.h>
#import "DKCommonTypes.h"

NS_ASSUME_NONNULL_BEGIN

@class DKDrawing, DKDrawingView, DKLayerGroup, DKDrawableObject, DKKnob, DKStyle, GCInfoFloater;

// generic layer class:

/** @brief Drawing layers are lightweight objects which represent a layer.

 Drawing layers are lightweight objects which represent a layer. They are owned by a DKDrawing which manages the
 stacking order and invokes the drawRect: method as needed. The other state variables control whether the layer is
 visible, locked, etc.
 
 \c DKDrawing will not ever call a \c drawRect: on a layer that returns \c NO for visible.
 
 If \c isOpaque returns YES, layers that are stacked below this one will not be drawn, even if they are visible. \c isOpaque
 returns \c NO by default.
 
 Locked layers should not be editable, but this must be enforced by subclasses, as this class contains no editing
 features. However, locked layers will never receive mouse event calls so generally this will be enough.
 
 As layers are retained by the drawing, this does not retain the drawing.
 
 By definition the bounds of the layer is the same as the bounds of the drawing.
 
 \c DKLayer does not implement a dragging destination but some subclasses do. \c NSDraggingDestination is declared to
 indicate likely handling of drag and drop operations by a layer instance.
*/
@interface DKLayer : NSObject <NSCoding, NSDraggingDestination, NSUserInterfaceValidations, DKKnobOwner> {
@private
	NSString* m_name; // layer name
	NSColor* m_selectionColour; // colour preference for selection highlights in this layer
	DKKnob* m_knobs; // knobs helper object if set - normally nil to defer to drawing
	BOOL m_knobsAdjustToScale; // YES if knobs allow for the view scale
	BOOL m_visible; // is the layer visible?
	BOOL m_locked; // is the layer locked?
	BOOL m_printed; // is the layer drawn when printing?
	BOOL mRulerMarkersEnabled; // YES to pass ruler marker updates to enclosing group, NO to ignore
	GCInfoFloater* m_infoWindow; // info window instance that can be used by client objects as they wish
	DKLayerGroup* __weak m_groupRef; // group we are contained by (or drawing)
	BOOL m_clipToInterior; // YES to clip drawing to inside the interior region
	NSMutableDictionary* mUserInfo; // metadata
	NSUInteger mReserved[3]; // unused
	NSString* mLayerUniqueKey; // unique ID for the layer
	CGFloat mAlpha; // alpha value applied to layer as a whole
}

/** @brief Returns the list of colours used for supplying the selection colours

 The list is used to supply colours in rotation when new layers are instantiated.
 If never specifically set, this returns a very simple list of basic colours which is what DK has
 traditionally used.
 @return an array containing NSColor objects
 */
@property (class, copy, null_resettable) NSArray<NSColor*>* selectionColours;
+ (nullable NSColor*)selectionColourForIndex:(NSUInteger)index;

- (instancetype)init NS_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder*)coder NS_DESIGNATED_INITIALIZER;

// owning drawing:

/** @brief Returns the drawing that the layer belongs to

 The drawing is the root object in a layer hierarchy, it overrides -drawing to return self, which is
 how this works
 @return the layer's owner drawing
 */
@property (readonly, strong) DKDrawing* drawing;

/** @brief Called when the drawing's undo manager is changed - this gives objects that cache the UM a chance
 to update their references

 The default implementation does nothing - override to make something of it
 @param um the new undo manager
 */
- (void)drawingHasNewUndoManager:(NSUndoManager*)um;

/** @brief Called when the drawing's size is changed - this gives layers that need to know about this a
 direct notification

 If you need to know before and after sizes, you'll need to subscribe to the relevant notifications.
 @param sizeVal the new size of the drawing - extract -sizeValue.
 */
- (void)drawingDidChangeToSize:(NSValue*)sizeVal;
- (void)drawingDidChangeMargins:(NSValue*)newInterior;

/** @brief Obtains the undo manager that is handling undo for the drawing and hence, this layer
 @return the undo manager in use
 */
@property (nonatomic, strong) NSUndoManager* undoManager;

/** @brief Notifies the layer that it or a group containing it was added to a drawing.

 This can be used to perform additional setup that requires knowledge of the drawing such as its
 size. The default method does nothing - override to use.
 @param aDrawing the drawing that added the layer
 */
- (void)wasAddedToDrawing:(DKDrawing*)aDrawing;

// layer group hierarchy:

/** @brief Sets the group that the layer is contained in - called automatically when the layer is added to a group

 The group retains this, so the group isn't retained here
 */
@property (weak, nullable) DKLayerGroup* layerGroup;

/** @brief Gets the layer's index within the group that the layer is contained in

 If the layer isn't in a group yet, result is 0. This is intended for debugging mostly.
 @return an integer, the layer's index
 */
@property (readonly) NSUInteger indexInGroup;

/** @brief Determine whether a given group is the parent of this layer, or anywhere above it in the hierarchy

 Intended to check for absurd operations, such as moving a parent group into one of its own children.
 @param aGroup a layer group
 @return YES if the group sits above this in the hierarchy, NO otherwise
 */
- (BOOL)isChildOfGroup:(DKLayerGroup*)aGroup;

/** @brief Returns the hierarchical level of this layer, i.e. how deeply nested it is

 Layers in the root group return 1. A layer's level is its group's level + 1
 @return the layer's level
 */
@property (readonly) NSUInteger level;

// drawing:

/** @brief Main entry point for drawing the layer and its contents to the drawing's views.

 Can be treated as the similar NSView call - to optimise drawing you can query the view that's doing
 the drawing and use calls such as needsToDrawRect: etc. Will not be called in
 cases where the layer is not visible, so you don't need to test for that. Must be overridden.
 @param rect the overall area being updated
 @param aView the view doing the rendering
 */
- (void)drawRect:(NSRect)rect inView:(nullable DKDrawingView*)aView;

/** @brief Is the layer opaque or transparent?

 Can be overridden to optimise drawing in some cases. Layers below an opaque layer are skipped
 when drawing, so if you know your layer is opaque, return YES to implement the optimisation.
 The default is NO, layers are considered to be transparent.
 @return whether to treat the layer as opaque or not
 */
@property (readonly, getter=isOpaque) BOOL opaque;

/** @brief Flags the whole layer as needing redrawing

 Always use this method instead of trying to access the view directly. This ensures that all attached
 views get refreshed correctly.
 @param update flag whether to update or not
 */
- (void)setNeedsDisplay:(BOOL)update;

/** @brief Flags part of a layer as needing redrawing

 Always use this method instead of trying to access the view directly. This ensures that all attached
 views get refreshed correctly.
 @param rect the area that needs to be redrawn
 */
- (void)setNeedsDisplayInRect:(NSRect)rect;

/** @brief Marks several areas for update at once

 Several update optimising methods return sets of rect values, this allows them to be processed
 directly.
 @param setOfRects a set containing NSValues with rect values
 */
- (void)setNeedsDisplayInRects:(NSSet<NSValue*>*)setOfRects NS_REFINED_FOR_SWIFT;

/** @brief Marks several areas for update at once

 Several update optimising methods return sets of rect values, this allows them to be processed
 directly.
 @param setOfRects a set containing NSValues with rect values
 @param padding the width and height will be added to EACH rect before invalidating
 */
- (void)setNeedsDisplayInRects:(NSSet<NSValue*>*)setOfRects withExtraPadding:(NSSize)padding NS_REFINED_FOR_SWIFT;

/** @brief Called before the layer starts drawing its content

 Can be used to hook into the start of drawing - by default does nothing
 */
- (void)beginDrawing;

/** @brief Called after the layer has finished drawing its content

 Can be used to hook into the end of drawing - by default does nothing
 */
- (void)endDrawing;

/** @brief Sets the colour preference to use for selected objects within this layer

 Different layers may wish to have a different colour for selections to help the user tell which
 layer they are working in. The layer doesn't enforce this - it's up to objects to make use of
 this provided colour where necessary.
 */
@property (nonatomic, strong, nullable) NSColor* selectionColour;

/** @brief Returns an image of the layer a the given size

 While the image has the size passed, the rendered content will have the same aspect ratio as the
 drawing, scaled to fit. Areas left outside of the drawn portion are transparent.
 @return an image of this layer only
 */
- (NSImage*)thumbnailImageWithSize:(NSSize)size;
/** @brief Returns an image of the layer at the default size
 @return an image of this layer only
 */
- (NSImage*)thumbnail;

/** @brief Returns the content of the layer as a pdf

 By default the pdf contains the entire layer's visible content exactly as drawn to a printer.
 @return NSData containing the pdf representation of the layer and its contents
 */
- (NSData*)pdf;

/** @brief Writes the content of the layer as a pdf to a nominated pasteboard

 Becomes the new pasteboard owner and removes any existing declared types
 @param pb the pasteboard
 @return YES if written OK, NO otherwise
 */
- (BOOL)writePDFDataToPasteboard:(NSPasteboard*)pb;

/** @brief Returns the layer's content as a transparent bitmap having the given DPI.

 A dpi of 0 uses the default, which is 72 dpi. The image pixel size is calculated from the drawing
 size and the dpi. The layer is imaged onto a transparent background with alpha.
 @param dpi image resolution in dots per inch
 @return the bitmap
 */
- (NSBitmapImageRep*)bitmapRepresentationWithDPI:(NSUInteger)dpi;

/** @brief Sets whether drawing is limited to the interior area or not

 Default is NO, so drawings show in the margins.
 Set to \c YES to limit drawing to the interior, \c NO to allow drawing to be visible in the margins.
 */
@property (nonatomic) BOOL clipsDrawingToInterior;

/** @brief Sets the alpha level for the layer

 Default is 1.0 (fully opaque objects). Note that alpha must be implemented by a layer's
 \c -drawRect:inView: method to have an actual effect, and unless compositing to a CGLayer or other
 graphics surface, may not have the expected effect (just setting the context's alpha before
 drawing renders each individual object with the given alpha, for example).
 */
@property (nonatomic) CGFloat alpha;

// managing ruler markers:

- (void)updateRulerMarkersForRect:(NSRect)rect;
- (void)hideRulerMarkers;
@property BOOL rulerMarkerUpdatesEnabled;

// states:

/** @brief Whether the layer is locked or not

 A locked layer will be drawn but cannot be edited. In case the layer's appearance changes
 according to this state change, a refresh is performed.
 Set to \c YES to lock, \c NO to unlock.
 */
@property BOOL locked;

/** @brief Whether the layer is visible or not.

 Invisible layers are neither drawn nor can be edited.
 Also returns \c NO if the layer's group is not visible.
 Set to \c YES to show the layer, \c NO to hide it.
 */
@property BOOL visible;

/** @brief Is the layer the active layer?
 @return YES if the active layer, NO otherwise
 */
@property (readonly, getter=isActive) BOOL active;

/** @brief Returns whether the layer is locked or hidden

 Locked or hidden layers cannot usually be edited.
 Is \c YES if locked or hidden, \c NO if unlocked and visible
 */
@property (readonly) BOOL lockedOrHidden;

/** @brief Sets the user-readable name of the layer

 Layer names are a convenience for the user, and can be displayed by a user interface. The name is
 not significant internally. This copies the name passed for safety.
 */
@property (nonatomic, copy) NSString* layerName;

// user info support

- (void)addUserInfo:(NSDictionary<NSString*, id>*)info;

/** @brief Return the attached user info

 The user info is returned as a mutable dictionary (which it is), and can thus have its contents
 mutated directly for certain uses. Doing this cannot cause any notification of the status of
 the object however.
 @return the user info
 */
@property (copy) NSMutableDictionary<NSString*, id>* userInfo;

/** @brief Return an item of user info
 @param key the key to use to refer to the item
 @return the user info item
 */
- (nullable id)userInfoObjectForKey:(NSString*)key;
- (void)setUserInfoObject:(id)obj forKey:(NSString*)key;

/** @brief Returns the layer's unique key
 @return the unique key
 */
@property (readonly, copy) NSString* uniqueKey;

// print this layer?

/** @brief Whether the layer should be part of the printed output or not

 Some layers won't want to be printed - guides for example. Override this to return NO if you
 don't want the layer to be printed. By default layers are printed.
 Set to \c YES to draw to printer, \c NO to suppress drawing on the printer.
 */
@property BOOL shouldDrawToPrinter;

// becoming/resigning active:

/** @brief Returns whether the layer can become the active layer

 The default is YES. Layers may override this and return \c NO if they do not want to ever become active
 Is \c YES if the layer can become active, \c NO to not become active
 */
@property (readonly) BOOL layerMayBecomeActive;

/** @brief The layer was made the active layer by the owning drawing

 Layers may want to know when their active state changes. Override to make use of this.
 */
- (void)layerDidBecomeActiveLayer;

/** @brief The layer is no longer the active layer

 Layers may want to know when their active state changes. Override to make use of this.
 */
- (void)layerDidResignActiveLayer;

// permitting deleton:

/** @brief Return whether the layer can be deleted

 This setting is intended to be checked by UI-level code to prevent deletion of layers within the UI.
 It does not prevent code from directly removing the layer.
 @return \c YES if layer can be deleted, override to return \c NO to prevent this
 */
@property (readonly) BOOL layerMayBeDeleted;

// mouse event handling:

/** @brief Should the layer automatically activate on a click if the view has this behaviour set?

 Override to return NO if your layer type should not auto activate. Note that auto-activation also
 needs to be set for the view. The event is passed so that a sensible decision can be reached.
 @param event the event (usually a mouse down) of the view that is asking
 @return YES if the layer is unlocked, NO otherwise
 */
- (BOOL)shouldAutoActivateWithEvent:(NSEvent*)event;

/** @brief Detect whether the layer was "hit" by a point.

 This is used to implement automatic layer activation when the user clicks in a view. This isn't
 always the most useful behaviour, so by default this returns NO. Subclasses can override to refine
 the hit test appropriately.
 @param p the point to test
 @return YES if the layer was hit, NO otherwise
 */
- (BOOL)hitLayer:(NSPoint)p;

/** @brief Detect what object was hit by a point.

 Layers that support objects implement this meaningfully. A non-object layer returns nil which
 simplifies the design of certain tools that look for targets to operate on, without the need
 to ascertain the layer class first.
 @param p the point to test
 @return the object hit, or nil
 */
- (nullable DKDrawableObject*)hitTest:(NSPoint)p;

/** @brief The mouse went down in this layer

 Override to respond to the event. Note that where tool controllers and tools are used, these
 methods may never be called, as the tool will operate on target objects within the layer directly.
 @param event the original mouseDown event
 @param view the view which responded to the event and passed it on to us
 */
- (void)mouseDown:(NSEvent*)event inView:(NSView*)view;

/**
 Subclasses must override to be notified of mouse dragged events
 @param event the original mouseDragged event
 @param view the view which responded to the event and passed it on to us
 */
- (void)mouseDragged:(NSEvent*)event inView:(NSView*)view;

/**
 Override to respond to the event
 @param event the original mouseUpevent
 @param view the view which responded to the event and passed it on to us
 */
- (void)mouseUp:(NSEvent*)event inView:(NSView*)view;

/** @brief Respond to a change in the modifier key state

 Is passed from the key view to the active layer
 @param event the event
 */
- (void)flagsChanged:(NSEvent*)event;

/** @brief Returns the view which is either currently drawing the layer, or the one that mouse events are
 coming from

 This generally does the expected thing. If you're drawing, it returns the view that's doing the drawing
 original event in question. At any other time it will return nil. Wherever possible you should
 use the view parameter that is passed to you rather than use this.
 @return the currently "important" view
 */
- (nullable NSView*)currentView;

/** @brief Returns the cursor to display while the mouse is over this layer while it's active

 Subclasses will usually want to override this and provide a cursor appropriate to the layer or where
 the mouse is within it, or which tool has been attached.
 @return the desired cursor
 */
- (NSCursor*)cursor;

/** @brief Return a rect where the layer's cursor is shown when the mouse is within it

 By default the cursor rect is the entire interior area.
 @return the cursor rect
 */
@property (readonly) NSRect activeCursorRect;

/** @brief Allows a contextual menu to be built for the layer or its contents

 By default this returns nil, resulting in nothing being displayed. Subclasses can override to build
 a suitable menu for the point where the layer was clicked.
 @param theEvent the original event (a right-click mouse event)
 @param view the view that received the original event
 @return a menu that will be displayed as a contextual menu
 */
- (nullable NSMenu*)menuForEvent:(NSEvent*)theEvent inView:(NSView*)view;

// supporting per-layer knob handling - default defers to the drawing as before

/** @brief Returns the attached selection knobs helper object
 
 Selection appearance can be customised for this drawing by setting up the knobs object or subclassing
 it. This object is propagated down to all objects below this in the system to draw their selection.
 See also: -setSelectionColour, -selectionColour.
 If custom knobs have been set for the layer, they are returned. Otherwise, the knobs for the group
 or ultimately the drawing will be returned.
 @return the attached knobs object
 */
@property (nonatomic, strong) DKKnob* knobs;
/** @brief Sets whether selection knobs should scale to compensate for the view scale. default is YES.
 
 In general it's best to scale the knobs otherwise they tend to overlap and become large at high
 zoom factors, and vice versa. The knobs objects itself decides exactly how to perform the scaling.
 @param ka YES to set knobs to scale, NO to fix their size.
 @deprecated Method was misspelled, use \c -setKnobsShouldAdjustToViewScale instead.
 */
- (void)setKnobsShouldAdustToViewScale:(BOOL)ka API_DEPRECATED_WITH_REPLACEMENT("setKnobsShouldAdjustToViewScale", macosx(10.0, 10.6));

/** @brief Sets whether selection knobs should scale to compensate for the view scale. default is YES.
 
 The default setting is YES, knobs should adjust to scale.
 In general, it's best to scale the knobs, otherwise they tend to overlap and become large at high
 zoom factors, and vice versa. The knobs objects itself decides exactly how to perform the scaling.
 Set to \c YES to set knobs to scale, \c NO to fix their size.
 */
@property (nonatomic) BOOL knobsShouldAdjustToViewScale;

// pasteboard types for drag/drop etc:

/** @brief Return the pasteboard types this layer is able to receive in a given operation (drop or paste)
 @param op the kind of operation we need pasteboard types for
 @return an array of pasteboard types
 they can handle and also implement the necessary parts of the NSDraggingDestination protocol
 just as if they were a view.
 */
- (nullable NSArray<NSPasteboardType>*)pasteboardTypesForOperation:(DKPasteboardOperationType)op;

/** @brief Tests whether the pasteboard has any of the types the layer is interested in receiving for the given
 operation
 @param pb the pasteboard
 @param op the kind of operation we need pasteboard types for
 @return YES if the pasteboard has any of the types of interest, otherwise NO
 */
- (BOOL)pasteboard:(NSPasteboard*)pb hasAvailableTypeForOperation:(DKPasteboardOperationType)op;

// style utilities (implemented by subclasses such as DKObjectOwnerLayer)

/** @brief Return all of styles used by the layer

 Override if your layer uses styles
 @return nil
 */
- (nullable NSSet<DKStyle*>*)allStyles;

/** @brief Return all of registered styles used by the layer

 Override if your layer uses styles
 @return nil
 */
- (nullable NSSet<DKStyle*>*)allRegisteredStyles;

/** @brief Substitute styles with those in the given set

 Subclasses may implement this to replace styles they use with styles from the set that have matching
 keys. This is an important step in reconciling the styles loaded from a file with the existing
 registry. Implemented by DKObjectOwnerLayer, etc. Layer groups also implement this to propagate
 the change to all sublayers.
 @param aSet a set of style objects
 */
- (void)replaceMatchingStylesFromSet:(NSSet<DKStyle*>*)aSet;

// info window utilities:

/** @brief Displays a small floating info window near the point p containg the string.

 The window is shown near the point rather than at it. Generally the info window should be used
 for small, dynamically changing and temporary information, like a coordinate value. The background
 colour is initially set to the layer's selection colour
 @param str a pre-formatted string containg some information to display
 @param p a point in local drawing coordinates
 */
- (void)showInfoWindowWithString:(NSString*)str atPoint:(NSPoint)p;

/** @brief Hides the info window if it's visible
 */
- (void)hideInfoWindow;

/** @brief Sets the background colour of the small floating info window
 @param colour a colour for the window
 */
- (void)setInfoWindowBackgroundColour:(NSColor*)colour;

// user actions:

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)lockLayer:(nullable id)sender;

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)unlockLayer:(nullable id)sender;

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)toggleLayerLock:(nullable id)sender;

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)showLayer:(nullable id)sender;

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)hideLayer:(nullable id)sender;

/**
 User interface level method can be linked to a menu or other appropriate UI widget
 @param sender the sender of the action
 */
- (IBAction)toggleLayerVisible:(nullable id)sender;

/**
 Debugging method
 @param sender the sender of the action
 */
- (IBAction)logDescription:(nullable id)sender;
- (IBAction)copy:(nullable id)sender;

@end

@interface DKLayer (OptionalMethods)

- (void)mouseMoved:(NSEvent*)event inView:(NSView*)view;

@end

extern NSNotificationName const kDKLayerLockStateDidChange;
extern NSNotificationName const kDKLayerVisibleStateDidChange;
extern NSNotificationName const kDKLayerNameDidChange;
extern NSNotificationName const kDKLayerSelectionHighlightColourDidChange;

NS_ASSUME_NONNULL_END
