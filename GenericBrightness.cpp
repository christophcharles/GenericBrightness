#include "GenericBrightness.h"

#define super IOService

OSDefineMetaClassAndStructors(GenericBrightness, IOService)


IOService* GenericBrightness::probe( IOService *provider, SInt32 *score )
{
    if( !super::probe( provider, score )) return( 0 );
	return (this);
}

bool GenericBrightness::start( IOService * provider )
{	
	
	if(!provider || !super::start(provider))
		return false;
	
	BrightnessMethods = (IOACPIPlatformDevice *) provider;

	provider->joinPMtree(this);
	
	this->registerService(0);
	if (GetBrightnessLevels()==0) return false;
	IOLog("Acpi brightness levels:%d, lowest brightness:%d, highest brightness:%d \n ", (int)brightnessIndex-2, (int)GetValueFromArray(brightnessTable,2), (int)GetValueFromArray(brightnessTable, brightnessIndex-1) );
		
	BTWorkLoop = getWorkLoop();
	BTPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &GenericBrightness::BrightnessCheck));
	if (!BTWorkLoop || !BTPollTimer || (kIOReturnSuccess != BTWorkLoop->addEventSource(BTPollTimer))) 
	{ 
		IOLog("Timer not working\n"); return false;
	}
	
	BrightnessCheck(); 
	return true;	
}

OSDictionary * getDisplayParams(IODisplay * display){
    OSDictionary * params = NULL;
    if(display && (display->getProperty("IODisplayGUID") != 0))
    {
        params = OSDynamicCast(OSDictionary, display->getProperty("IODisplayParameters"));
    }
    return params;
}

IODisplay * GenericBrightness::getDisplay(void){
    IODisplay * displayRes;
    OSIterator * displayList;
    displayList = getMatchingServices(serviceMatching("IOBacklightDisplay"));
    if (displayList)
    {
        IOService *obj = NULL;
        while( (obj = (IOService *) displayList->getNextObject()) )
        {
            displayRes = OSDynamicCast( IOBacklightDisplay, obj);
        }
        displayList->release();
        displayList = NULL;
    }
    return displayRes;
}

void GenericBrightness::BrightnessCheck(void)
{
	
	if (!displayParams || !display || display != getDisplay() || getDisplayParams(display)!=displayParams || !IODisplay::getIntegerRange(displayParams, gIODisplayBrightnessKey,
													  &fCurrentBrightness, &fMinBrightness, &fMaxBrightness)) 
	{
		IOLog("We still don't have brightness entry in ioreg... waiting...\n");
        display = getDisplay();
        if(display){
            displayParams = getDisplayParams(display);
 		}
		BTPollTimer->setTimeoutMS(10000);
	}
	else {
		BTPollTimer->setTimeoutMS(1000);
	}
	
	if (fLastBrightness != fCurrentBrightness)
	{
		fLastBrightness = fCurrentBrightness;
//		IOLog("brightnessmin  %d, brightnessmax %d, currentbrightness %d\n", (int)fMinBrightness, (int)fMaxBrightness, (int)fCurrentBrightness);
		
//		SInt32	fHardBrightness=fCurrentBrightness/(fMaxBrightness/(brightnessIndex-2))+2;
//		SInt32	fHardBrightness=((fCurrentBrightness*(brightnessIndex-3))/fMaxBrightness)+2;

//		if (fHardBrightness>brightnessIndex-1) fHardBrightness=brightnessIndex-1;
//		IOLog("Index of brightness table:%d , brightness to call:%d \n",  (int)fHardBrightness, (int)GetValueFromArray(brightnessTable,fHardBrightness));
		OSObject * param = OSNumber::withNumber(GetValueFromArray(brightnessTable,((fCurrentBrightness*(brightnessIndex-3))/fMaxBrightness)+2),8);

		SetBrightness("_BCM", param);		
	}				
}

IOReturn GenericBrightness::SetBrightness(const char * method, OSObject * param)
{
	OSObject * acpi;
	if (BrightnessMethods->evaluateObject(method, &acpi, &param, 1) != kIOReturnSuccess)
	{
		IOLog("%s: No object of method %s\n", this->getName(), method);
		return 0;
	}
	return 1;
}

IOReturn GenericBrightness::GetBrightnessLevels(void)
{
	OSObject * brightnessLevels;
	
	
	if (kIOReturnSuccess == BrightnessMethods->evaluateObject("_BCL", &brightnessLevels)) {
		brightnessTable = OSDynamicCast(OSArray, brightnessLevels);
		brightnessIndex = brightnessTable->getCount();
		return brightnessTable->getCount();
	} else {
		return 0;
	}
}

UInt32 GenericBrightness::GetValueFromArray(OSArray * array, UInt8 index) {
	OSObject * object = array->getObject(index);
	if (object && (OSTypeIDInst(object) == OSTypeID(OSNumber))) {
		OSNumber * number = OSDynamicCast(OSNumber, object);
		if (number) return number->unsigned32BitValue();
	}
	return -1;
}


bool GenericBrightness::init(OSDictionary *properties)
{    
	return super::init(properties);
}

void GenericBrightness::stop( IOService * provider )
{	
	if( BTPollTimer ) {
		BTWorkLoop->removeEventSource(BTPollTimer);
		BTPollTimer->release();
	}	

	if( BTWorkLoop ) {
		BTWorkLoop->release();
	}
	
    super::stop( provider );

}

void GenericBrightness::free ()
{
	super::free ();
}
