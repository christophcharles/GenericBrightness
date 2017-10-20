#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "IODisplay.h"


class GenericBrightness : public IOService
{
	OSDeclareDefaultStructors(GenericBrightness)


	
private:
	IOACPIPlatformDevice * BrightnessMethods;

	
protected:
	SInt32	fCurrentBrightness;
    SInt32	fMinBrightness;
    SInt32	fMaxBrightness;
	SInt32	fLastBrightness;
	SInt32	brightnessIndex;
	OSDictionary * displayParams;
    IODisplay * display;
    OSArray * brightnessTable;
    
	IOWorkLoop *		 BTWorkLoop;
	IOTimerEventSource * BTPollTimer;
	
//	IODisplayConnect *			fConnection;
//    IONotifier *			fNotifier;

	
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
	virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	virtual void		BrightnessCheck( void );
    virtual IODisplay * getDisplay(void);

	IOReturn SetBrightness (const char * method, OSObject * param);
	IOReturn GetBrightnessLevels(void);
	UInt32 GetValueFromArray(OSArray * array, UInt8 index);


};