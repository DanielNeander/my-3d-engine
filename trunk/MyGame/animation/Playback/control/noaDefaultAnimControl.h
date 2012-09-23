#ifndef NO_PLAYBACK_DEFAULTANIMCONTROL_H
#define NO_PLAYBACK_DEFAULTANIMCONTROL_H

//#include <Animation/NOAnimBinding.h>
#include <Animation/noaAnimBinding.h>
#include <Animation/Playback/control/noaAnimControl.h>
#include <Math/Vector.h>


class noaDefaultAnimControl : public noaAnimControl
{
public:
	noaDefaultAnimControl( const noaAnimBinding* binding, hkBool startEaseIn = true, 
		hkInt32 maxCycles = -1);

	void Update( hkReal stepDelta );
	virtual void GetFutureTime( hkReal stepDelta, hkReal& localTimeOut, int& loopsOut) const;

		/// Return the master weight for the control
		inline hkReal GetMasterWeight() const;

			/// Set the master weight for this playing animation
		inline void SetMasterWeight( hkReal weight );

			/// Return the current playback speed
		inline hkReal GetPlaybackSpeed() const;

			/// Set the playback speed.
			/// If speed is set to a negative value the animation will run backwards.
		inline void SetPlaybackSpeed( hkReal speed );

			/*
			 * Loop counting
			 */

			/// Return how many times this control has looped passed the end of the animation - never reset unless by user using setOverflowCount().
		inline hkUint32 GetOverflowCount() const;

			/// Reset (to zero) how many times this control has looped passed the end of the animation.
		inline void SetOverflowCount( hkUint32 count );

			/// Return how many times this control has looped passed the start of the animation - never reset unless by user using setUnderflowCount().
		inline hkUint32 GetUnderflowCount() const;

			/// Reset (to zero) how many times this control has looped passed the start of the animation.
		inline void SetUnderflowCount( hkUint32 count );

			/*
			 * Ease Curve Control
			 */

			/// Set the ease in curve. The curve is a Bezier defined by
			/// B(t) = t^3(-p0 + 3p1 - 3p2 + p3) + t^2(3p0 - 6p1 + 3p2) + t^1(-3p0 + 3p1) + t^0(p0)
		inline void SetEaseInCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3);

			/// Set the ease out curve. The curve is a Bezier defined by
			/// B(t) = t^3(-p0 + 3p1 - 3p2 + p3) + t^2(3p0 - 6p1 + 3p2) + t^1(-3p0 + 3p1) + t^0(p0)
		inline void SetEaseOutCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3);

			/// Ease in the control according to the curve.
			/// \param duration The length of time to perform the ease
			/// \return The time at which it will be fully eased in (may not be time specified if the control is already in the middle of easing).
		inline hkReal EaseIn(hkReal duration);

			/// Ease out the control according to the curve.
			/// \param duration The length of time to perform the ease
			/// \return The time at which it will be fully eased out (may not be time specified if the control is already in the middle of easing).
		inline hkReal EaseOut(hkReal duration);


	/// Ease status.
	enum EaseStatus
	{
		/// easeIn has been called, and ease is not yet complete
		EASING_IN,
		/// easeIn was called and ease has completed (or controller constructed in eased in state)
		EASED_IN,
		/// easeOut has been called, and ease is not yet complete
		EASING_OUT,
		/// easeOut was called and ease has completed (or controller constructed in eased out state)
		EASED_OUT
	};

	/// Returns the status of the easing for this control
	inline enum EaseStatus GetEaseStatus() const;

		/*
			 * Cropping
			 */

			/// Set the amount (in local seconds) to crop the start of the animation.
			/// This number should always be between 0 and the duration of the animation
		inline void SetCropStartAmountLocalTime( hkReal cropStartAmountLocalTime );

			/// Set the amount (in local seconds) to crop the end of the animation.
			/// This number should always be between 0 and the duration of the animation
			/// For example, to crop the last half second from an animation of any length, 
			/// pass in 0.5f.
		inline void SetCropEndAmountLocalTime( hkReal cropEndAmountLocalTime );

			/// Get the amount (in local seconds) to crop the start of the animation.
		inline hkReal GetCropStartAmountLocalTime();

			/// Get the amount (in local seconds) to crop the end of the animation.
		inline hkReal GetCropEndAmountLocalTime();

protected:
	hkReal	m_masterWeight;
	hkReal	m_playbackSpeed;
	hkUint32 m_overflowCount;
	hkUint32 m_underflowCount;
	hkInt32 m_maxCycles;

	// Ease controls
	noVec4 m_easeInCurve;
	noVec4 m_easeOutCurve;
	hkReal m_easeInvDuration;
	hkReal m_easeT;
	EaseStatus m_easeStatus;

	hkReal m_cropStartAmountLocalTime;
	hkReal m_cropEndAmountLocalTime;
};

inline void noaDefaultAnimControl::GetFutureTime( hkReal stepDelta, hkReal& localTimeOut, int& loopsOut ) const
{

}

inline hkReal noaDefaultAnimControl::GetMasterWeight() const
{
	return m_masterWeight;
}

inline void noaDefaultAnimControl::SetMasterWeight( hkReal weight )
{
	m_masterWeight = weight;
}

inline hkReal noaDefaultAnimControl::GetPlaybackSpeed() const
{
	return m_playbackSpeed;
}

inline void noaDefaultAnimControl::SetPlaybackSpeed( hkReal speed )
{
	m_playbackSpeed = speed;
}

inline hkUint32 noaDefaultAnimControl::GetOverflowCount() const
{
	return m_overflowCount;
}

inline void noaDefaultAnimControl::SetOverflowCount( hkUint32 count )
{
	m_overflowCount = count;
}

inline hkUint32 noaDefaultAnimControl::GetUnderflowCount() const
{
	return m_underflowCount;
}

inline void noaDefaultAnimControl::SetUnderflowCount( hkUint32 count )
{
	m_underflowCount = count;
}

inline void noaDefaultAnimControl::SetEaseInCurve( hkReal y0, hkReal y1, hkReal y2, hkReal y3 )
{
	m_easeInCurve.Set(y0, y1, y2, y3);
}

inline void noaDefaultAnimControl::SetEaseOutCurve( hkReal y0, hkReal y1, hkReal y2, hkReal y3 )
{
	m_easeOutCurve.Set(y0, y1, y2, y3);
}

inline hkReal noaDefaultAnimControl::EaseIn( hkReal duration )
{
	const hkReal denominator = duration; // avoid sn compiler warning

	// Test if the control is currently easing out
	if ( m_easeStatus == EASING_OUT || m_easeStatus == EASED_OUT )
	{
		// "Mirror" the amount of time left for easing
		m_easeT = 1.0f - m_easeT;
	}

	m_easeInvDuration = (duration > noMath::FLT_EPSILON) ? (1.0f / denominator) : HK_REAL_MAX;
	m_easeStatus = ( m_easeT == 1.0f ) ? EASED_IN : EASING_IN;

	return duration * (1.0f - m_easeT);
}

inline hkReal noaDefaultAnimControl::EaseOut( hkReal duration )
{
	const hkReal denominator = duration; // avoid sn compiler warning

	// Test if the control is currently easing out
	if ( m_easeStatus == EASING_IN || m_easeStatus == EASED_IN )
	{
		// "Mirror" the amount of time left for easing
		m_easeT = 1.0f - m_easeT;
	}

	m_easeInvDuration = (duration > noMath::FLT_EPSILON) ? (1.0f / denominator) : HK_REAL_MAX;
	m_easeStatus = ( m_easeT == 1.0f ) ? EASED_OUT : EASING_OUT;

	return duration * (1.0f - m_easeT);
}


inline noaDefaultAnimControl::EaseStatus noaDefaultAnimControl::GetEaseStatus() const
{
	return m_easeStatus;
}

inline void noaDefaultAnimControl::SetCropStartAmountLocalTime( hkReal cropStartAmountLocalTime )
{
	m_cropStartAmountLocalTime = cropStartAmountLocalTime;
}

inline void noaDefaultAnimControl::SetCropEndAmountLocalTime( hkReal cropEndAmountLocalTime )
{
	m_cropEndAmountLocalTime = cropEndAmountLocalTime;
}

inline hkReal noaDefaultAnimControl::GetCropStartAmountLocalTime()
{
	return m_cropStartAmountLocalTime;
}

inline hkReal noaDefaultAnimControl::GetCropEndAmountLocalTime()
{
	return m_cropEndAmountLocalTime;
}

#endif