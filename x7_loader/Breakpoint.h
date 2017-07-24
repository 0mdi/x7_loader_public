#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>

#include <stdio.h>
#include <Ntsecapi.h> 

typedef BOOL ( WINAPI * tGetThreadContext )( HANDLE hThread, LPCONTEXT lpContext);
typedef NTSTATUS (NTAPI *tNtSetContextThread)(HANDLE, LPCONTEXT);

tNtSetContextThread pNtSetContextThread;
tGetThreadContext oGetThreadContext;// not needed if you haven't hooked GetThreadContext

#define InitializeObjectAttributes( p, n, a, r, s ) {   \
	(p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
} 

typedef struct _OBJECT_ATTRIBUTES
{
	ULONG Length;
	PVOID RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;
	PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID
{
	DWORD UniqueProcess;
	DWORD UniqueThread;
} CLIENT_ID, *PCLIENT_ID; 

class CBreakpoint
{
    struct sBreakpoint
    {
        DWORD dwAddress, dwEip, dwCustomFilter;
        sBreakpoint( DWORD dw_Address, DWORD dw_Eip, DWORD dw_CustomFilter = 0 )
        {
            dwAddress = dw_Address;
            dwEip = dw_Eip;
            dwCustomFilter = dw_CustomFilter;
        }
    };
    std::vector<sBreakpoint> vBreakpoint;

public:

    CBreakpoint();

    DWORD GetMainThreadId( DWORD pID );

    void ApplyBreakpoint();
    void SetupThread(DWORD tid);

    void Clear();
    void AddBreakpoint( DWORD dwAddress, DWORD dwEip, PVECTORED_EXCEPTION_HANDLER pCustomFilter = 0 );
    void RemoveBreakpoint( DWORD dwAddress );

    LONG WINAPI UnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo );
};

extern CBreakpoint * Breakpoint;