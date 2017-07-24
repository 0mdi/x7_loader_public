/************************************************************************/
/* Author  :    Neeya Nanaa
 * Name    :    CWDDM.h
 * Usage   :    With this calss you are ablte to create an undetected D3D Hook
 * 
 * Credits :    NtKid & gamedeception.net
 * */
/************************************************************************/

#ifndef CWDDM_H
#define CWDDM_H


#include <Windows.h>
#include <d3d9.h>

typedef void (__stdcall *tRenderCallBack)(LPDIRECT3DDEVICE9);

#pragma region by NtKid
typedef enum
{
	cbAllocateCb = 6,
	cbDeallocateCb = 7,
	cbSetPriorityCb = 8,
	cbQueryResidencyCb = 9,
	cbSetDisplayModeCb = 10,
	cbPresentCb = 11,
	cbRenderCb = 12,
	cbLockCb = 13,
	cbUnlockCb = 14,
	cbEscapeCb = 15,
	cbCreateOverlayCb = 16,
	cbUpdateOverlayCb = 17,
	cbFlipOverlayCb = 18,
	cbDestroyOverlayCb = 19,
	cbCreateContextCb = 20,
	cbDestroyContextCb = 21,
	cbCreateSynchronizationObjectCb = 22,
	cbDestroySynchronizationObjectCb = 23,
	cbWaitForSynchronizationObjectCb = 24,
	cbSignalSynchronizationObjectCb = 25,
	cbSetAsyncCallbacksCb = 26,
	cbSetDisplayPrivateDriverFormatCb = 27,
}adpcallback_t;

typedef enum
{
	AdpSetRenderState,
	AdpUpdateWInfo,
	AdpValidateDevice,
	AdpSetTextureStageState,
	AdpAdpSetTexture,
	AdpSetPixelShader,
	AdpSetPixelShaderConst,
	AdpSetStreamSourceUm,
	AdpSetIndices,
	AdpSetIndicesUm,
	AdpDrawPrimitive,
	AdpDrawIndexedPrimitive,
	AdpDrawRectPatch,
	AdpDrawTriPatch,
	AdpDrawPrimitive2,
	AdpDrawIndexedPrimitive2,
	AdpVolBlt,
	AdpBufBlt,
	AdpTexBlt,
	AdpStateSet,
	AdpSetPriority,
	AdpClear,
	AdpUpdatePalette,
	AdpSetPalette,
	AdpSetVertexShaderConst,
	AdpMultiplyTransform,
	AdpSetTransform,
	AdpSetViewport,
	AdpSetZRange,
	AdpSetMaterial,
	AdpSetLight,
	AdpCreateLight,
	AdpDestroyLight,
	AdpSetClipPlane,
	AdpGetInfo,
	AdpLock,
	AdpUnlock,
	AdpCreateResource,
	AdpDestroyResource,
	AdpSetDisplayMode,
	AdpPresent,
	AdpFlush,
	AdpCreateVertexShaderFunc,
	AdpDeleteVertexShaderFunc,
	AdpSetVertexShaderFunc,
	AdpCreateVertexShaderDecl,
	AdpDeleteVertexShaderDecl,
	AdpSetVertexShaderDecl,
	AdpSetVertexShaderConstI,
	AdpSetVertexShaderConstB,
	AdpSetScissorRect,
	AdpSetStreamSource,
	AdpSetStreamSourceFreq,
	AdpSetConvolutionKernelMono,
	AdpComposeRects,
	AdpBlt,
	AdpColorFill,
	AdpDepthFill,
	AdpCreateQuery,
	AdpDestroyQuery,
	AdpIssueQuery,
	AdpGetQueryData,
	AdpSetRenderTarget,
	AdpSetDepthStencil,
	AdpGenerateMipSubLevels,
	AdpSetPixelShaderConstI,
	AdpSetPixelShaderConstB,
	AdpCreatePixelShader,
	AdpDeletePixelShader,
	AdpCreateDecodeDevice,
	AdpDestroyDecodeDevice,
	AdpSetDecodeRenderTarget,
	AdpDecodeBeginFrame,
	AdpDecodeEndFrame,
	AdpDecodeExecute,
	AdpDecodeExtensionExecute,
	AdpCreateVideoProcessDevice,
	AdpDestroyVideoProcessDevice,
	AdpVideoProcessBeginFrame,
	AdpVideoProcessEndFrame,
	AdpSetVideoProcessRenderTarget,
	AdpVideoProcessBlt,
	AdpCreateExtensionDevice,
	AdpDestroyExtensionDevice,
	AdpExtensionExecute,
	AdpCreateOverlay,
	AdpUpdateOverlay,
	AdpFlipOverlay,
	AdpGetOverlayColorControls,
	AdpSetOverlayColorControls,
	AdpDestroyOverlay,
	AdpDestroyDevice,
	AdpQueryResourceResidency,
	AdpOpenResource,
	AdpGetCaptureAllocationHandle,
	AdpCaptureToSysMem,
	AdpLockAsync,
	AdpUnlockAsync,
	AdpRename,
	//bellow is win7 only
	AdpCreateVideoProcessor,
	AdpSetVideoProcessBltState,
	AdpGetVideoProcessBltStatePrivate,
	AdpSetVideoProcessStreamState,
	AdpGetVideoProcessStreamStatePrivate,
	AdpVideoProcessBltHD,
	AdpDestroyVideoProcessor,
	AdpCreateAuthenticatedChannel,
	AdpAuthenticatedChannelKeyExchange,
	AdpQueryAuthenticatedChannel,
	AdpConfigureAuthenticatedChannel,
	AdpDestroyAuthenticatedChannel,
	AdpCreateCryptoSession,
	AdpCryptoSessionKeyExchange,
	AdpDestroyCryptoSession,  
	AdpEncryptionBlt,
	AdpGetPitch,
	AdpStartSessionKeyRefresh,
	AdpFinishSessionKeyRefresh,
	AdpGetEncryptionBltKey,    
	AdpDecryptionBlt,
	AdpResolveSharedResource,
}adpdev_t;

typedef enum D3DDDI_FLIPINTERVAL_TYPE
{
	D3DDDI_FLIPINTERVAL_IMMEDIATE,
	D3DDDI_FLIPINTERVAL_ONE,
	D3DDDI_FLIPINTERVAL_TWO,
	D3DDDI_FLIPINTERVAL_THREE,
	D3DDDI_FLIPINTERVAL_FOUR,
} D3DDDI_FLIPINTERVAL_TYPE;

typedef struct _D3DDDI_PRESENTFLAGS
{
	union
	{
		struct
		{
			UINT Blt:1;
			UINT ColorFill:1;
			UINT Flip:1;
			UINT Reserved:29;
		};
		UINT Value;
	};            
} D3DDDI_PRESENTFLAGS;

typedef struct _D3DDDIARG_PRESENT
{
	HANDLE hSrcResource;
	UINT SrcSubResourceIndex;
	HANDLE hDstResource;
	UINT DstSubResourceIndex;
	D3DDDI_PRESENTFLAGS Flags;
	D3DDDI_FLIPINTERVAL_TYPE FlipInterval;
} D3DDDIARG_PRESENT;

typedef struct _D3DDDIARG_DRAWINDEXEDPRIMITIVE
{
	D3DPRIMITIVETYPE PrimitiveType;
	INT BaseVertexIndex;
	UINT MinIndex;
	UINT NumVertices;
	UINT StartIndex;
	UINT PrimitiveCount;
}D3DDDIARG_DRAWINDEXEDPRIMITIVE;

typedef struct _D3DDDIARG_SETSTREAMSOURCE
{
	UINT Stream;
	HANDLE hVertexBuffer;
	UINT Offset;
	UINT Stride;
} D3DDDIARG_SETSTREAMSOURCE;

typedef struct _ADAPTER_STRUCT
{
	BYTE Unk0[32];
	PDIRECT3DDEVICE9 pDev;
	BYTE Unk1[896];
	PULONG AdapterCallback;
	PULONG AdapterFuncs;
}ADAPTER_STRUCT;

#pragma endregion

class CWDDM
{
public:
	//Singleton
	static CWDDM *getInstance();

	void init(HMODULE hD3d9, tRenderCallBack pRenderCallBack);

	tRenderCallBack m_pRenderCallBack;
	static IDirect3DDevice9* m_pDevice;

private:
	
	bool ReadSignature(DWORD dwAddress, BYTE *pbSignature, int iLenght);

	//Constructor & Deconstructor
	CWDDM();
	~CWDDM();

	//Singleton
	static CWDDM *m_Instance;
};


#endif