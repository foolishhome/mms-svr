;//begin==============================================================
;#ifndef __CMCARD_H__
;#define __CMCARD_H__

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )
              
LanguageNames=(Chinese=2052:MSG0052)

; // The following are the categories of events.
MessageId=0x1
SymbolicName=SYSTEM_CATEGORY
Language=Chinese
System Events
.

MessageId=0x2
SymbolicName=NETWORK_CATEGORY
Language=Chinese
Network Events
.

MessageId=0x3
SymbolicName=REDIS_CATEGORY
Language=Chinese
UI Events
.

MessageId=0x4
SymbolicName=DATABASE_CATEGORY
Language=Chinese
Database Events
.


; // The following are the message definitions.
MessageId=0x100
Severity=Error
Facility=Runtime
SymbolicName=MMS_COMMAND
Language=Chinese
yymms_svr %1.
.


;#endif  //__CMCARD_H__
;//end================================================================
