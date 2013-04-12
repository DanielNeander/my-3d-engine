// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RELOADABLE_BUNDLE_H
#define __RELOADABLE_BUNDLE_H

#include "../util/Notifier.h"

//namespace dingus {


/**
 *  Reloadable resource (usually bundle).
 *
 *  Is able to reload all the resources it contains. The application won't
 *  notice this, because it's dealing with proxies anyway. Reloading should
 *  preserve the resource proxy objects, only change the objects they refer to.
 */
class IReloadableBundle {
public:
	virtual ~IReloadableBundle() = 0 { }

	virtual void reload() = 0;
};


/**
 *  Manager for reloadable bundles.
 */
class CReloadableBundleManager : public IReloadableBundle, public CNotifier<IReloadableBundle> {
public:
	// IReloadableBundle
	virtual void reload() {
		int n = getListeners().size();
		for( int i = 0; i < n; ++i )
			getListeners()[i]->reload();
	}
};



//}; // namespace

#endif