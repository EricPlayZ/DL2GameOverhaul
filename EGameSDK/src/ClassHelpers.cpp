#include <EGSDK\ClassHelpers.h>

namespace EGSDK {
	namespace ClassHelpers {
		static volatile LONG disableVftableScanning = 0;

		bool IsVftableScanningDisabled() { return _InterlockedCompareExchange(&disableVftableScanning, 0, 0) != 0; }
		void SetIsVftableScanningDisabled(bool value) { _InterlockedExchange(&disableVftableScanning, value ? 1 : 0); }
	}
}