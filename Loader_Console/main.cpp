#include <Windows.h>
#include "..\Shared\PELoaderStub\PELoaderStub.h"
#include "..\Shared\PELoader\PELoader\PEImage.h"
#include "..\Shared\PELoader\PELoader\TlsResolver.h"
#include "..\Shared\PELoader\TlsCallbackProxy\TlsCallbackProxy.h"

PELoader::TlsResolver* _tlsResolver = nullptr;
PELoader::PEImage* _peImage = nullptr;
BOOL _entryPointCalled = FALSE;

void TlsCallbackProxy(PVOID hModule, DWORD dwReason, PVOID pContext)
{
  if (_entryPointCalled == FALSE) return;

  if (dwReason == DLL_THREAD_DETACH)
  {
    _tlsResolver->ExecuteCallbacks(_peImage, dwReason, pContext);
    _tlsResolver->ClearTlsData();
  }
  else if (dwReason == DLL_THREAD_ATTACH)
  {
    _tlsResolver->InitializeTlsData(_peImage);
    _tlsResolver->ExecuteCallbacks(_peImage, dwReason, pContext);
  }
  else if (dwReason == DLL_PROCESS_ATTACH)
  {
    _tlsResolver->ExecuteCallbacks(_peImage, dwReason, pContext);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    _tlsResolver->ExecuteCallbacks(_peImage, dwReason, pContext);
  }
}

void main()
{
  _tlsResolver = new PELoader::TlsResolver();

  PELoaderStub::PELoaderStub* peLoaderStub = new PELoaderStub::PELoaderStub();
  _peImage = peLoaderStub->Load(_tlsResolver);
  delete peLoaderStub;

  // Call entry point  
  LPVOID entryPoint = _peImage->GetEntryPoint();
  _entryPointCalled = TRUE;
  ((void(*)())(entryPoint))();
}