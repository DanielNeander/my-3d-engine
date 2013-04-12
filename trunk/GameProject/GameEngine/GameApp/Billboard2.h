#pragma once 

#include "Common.h"
#include "ColorValue.h"

class Billboard 
{
public:
	friend class BillboardSet;


protected:
	bool mOwnDimensions;
	bool mUseTexcoordRect;
	uint16 mTexcoordIndex;      // index into the BillboardSet array of texture coordinates
	FloatRect mTexcoordRect;    // individual texture coordinates
	Real mWidth;
	Real mHeight;
public:
	// Note the intentional public access to main internal variables used at runtime
	// Forcing access via get/set would be too costly for 000's of billboards
	noVec3 mPosition;
	// Normalised direction vector
	noVec3 mDirection;
	BillboardSet* mParentSet;
	ColourValue mColour;
	float mRotation;	// Radian

	Billboard();

        /** Default destructor.
        */
        ~Billboard();

        /** Normal constructor as called by BillboardSet.
        */
        Billboard(const noVec3& position, BillboardSet* owner, const ColourValue& colour = ColourValue::White);

        /** Get the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
        */
        const float& getRotation(void) const { return mRotation; }

        /** Set the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
        */
        void setRotation(const float& rotation);

        /** Set the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        void setPosition(const noVec3& position);

        /** Set the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        void setPosition(Real x, Real y, Real z);

        /** Get the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        const noVec3& getPosition(void) const;

        /** Sets the width and height for this billboard.
            @remarks
                Note that it is most efficient for every billboard in a BillboardSet to have the same dimensions. If you
                choose to alter the dimensions of an individual billboard the set will be less efficient. Do not call
                this method unless you really need to have different billboard dimensions within the same set. Otherwise
                just call the BillboardSet::setDefaultDimensions method instead.
        */
        void setDimensions(Real width, Real height);

        /** Resets this Billboard to use the parent BillboardSet's dimensions instead of it's own. */
        void resetDimensions(void) { mOwnDimensions = false; }
        /** Sets the colour of this billboard.
            @remarks
                Billboards can be tinted based on a base colour. This allows variations in colour irrespective of the
                base colour of the material allowing more varied billboards. The default colour is white.
                The tinting is effected using vertex colours.
        */
        void setColour(const ColourValue& colour);

        /** Gets the colour of this billboard.
        */
        const ColourValue& getColour(void) const;

        /** Returns true if this billboard deviates from the BillboardSet's default dimensions (i.e. if the
            Billboard::setDimensions method has been called for this instance).
            @see
                Billboard::setDimensions
        */
        bool hasOwnDimensions(void) const;

        /** Retrieves the billboard's personal width, if hasOwnDimensions is true. */
        Real getOwnWidth(void) const;

        /** Retrieves the billboard's personal width, if hasOwnDimensions is true. */
        Real getOwnHeight(void) const;

        /** Internal method for notifying the billboard of it's owner.
        */
        void _notifyOwner(BillboardSet* owner);

        /** Returns true if this billboard use individual texture coordinate rect (i.e. if the 
            Billboard::setTexcoordRect method has been called for this instance), or returns
            false if use texture coordinates defined in the parent BillboardSet's texture
            coordinates array (i.e. if the Billboard::setTexcoordIndex method has been called
            for this instance).
            @see
                Billboard::setTexcoordIndex()
                Billboard::setTexcoordRect()
        */
        bool isUseTexcoordRect(void) const { return mUseTexcoordRect; }

        /** setTexcoordIndex() sets which texture coordinate rect this billboard will use 
            when rendering. The parent billboard set may contain more than one, in which 
            case a billboard can be textured with different pieces of a larger texture 
            sheet very efficiently.
          @see
            BillboardSet::setTextureCoords()
          */
        void setTexcoordIndex(uint16 texcoordIndex);

        /** getTexcoordIndex() returns the previous value set by setTexcoordIndex(). 
            The default value is 0, which is always a valid texture coordinate set.
            @remarks
                This value is useful only when isUseTexcoordRect return false.
          */
        uint16 getTexcoordIndex(void) const { return mTexcoordIndex; }

        /** setTexcoordRect() sets the individual texture coordinate rect of this billboard
            will use when rendering. The parent billboard set may contain more than one, in
            which case a billboard can be textured with different pieces of a larger texture
            sheet very efficiently.
        */
        void setTexcoordRect(const FloatRect& texcoordRect);

        /** setTexcoordRect() sets the individual texture coordinate rect of this billboard
            will use when rendering. The parent billboard set may contain more than one, in
            which case a billboard can be textured with different pieces of a larger texture
            sheet very efficiently.
        */
        void setTexcoordRect(Real u0, Real v0, Real u1, Real v1);

        /** getTexcoordRect() returns the previous value set by setTexcoordRect(). 
            @remarks
                This value is useful only when isUseTexcoordRect return true.
        */
        const FloatRect& getTexcoordRect(void) const { return mTexcoordRect; }

};
