// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>
#include <tpie/backtrace.h>
#include <tpie/tpie_log.h>

///////////////////////////////////////////////////////////////////////////////
/// \file backtrace.cpp Implementation of backtrace.h.
/// \sa backtrace.h
///////////////////////////////////////////////////////////////////////////////

namespace {
	void backtrace_begin(std::ostream & out) {
		out << "====================> Backtrace <======================" << std::endl;
	}

#if defined(WIN32) || defined(__CYGWIN__)
	void backtrace_item(std::ostream & out, size_t addr, const char * name, const char * file, int line) {
		if (name[0] != '\0') out << name;
		else if (addr != 0) out << addr;
		else out << "Unknown";
		if (file[0] != '\0') {
			out << " (" << file;
			if (line != -1) out << ":" << line;
			out << ")";
		}
		out << std::endl;
	}
#endif

	void backtrace_end(std::ostream & out) {
		out << "=======================================================" << std::endl;
	}
}

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include <WinBase.h>
#include <dbghelp.h>
#include <tchar.h>
#include <sstream>
#include <TlHelp32.h>

namespace {
typedef BOOL (__stdcall * stackWalk_t)( 
	DWORD MachineType, 
	HANDLE hProcess,
	HANDLE hThread, 
	LPSTACKFRAME64 StackFrame, 
	PVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );

typedef BOOL (__stdcall *getLineFromAddr_t)(IN HANDLE hProcess, IN DWORD64 dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line);

typedef BOOL (__stdcall *getSymFromAddr_t)(IN HANDLE hProcess, IN DWORD64 dwAddr,
	OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );

typedef DWORD (__stdcall WINAPI *undecorateSymbolName_t)(PCSTR DecoratedName, PSTR UnDecoratedName,
	DWORD UndecoratedLength, DWORD Flags);

typedef PVOID (__stdcall *functionTableAccess_t)(HANDLE hProcess, DWORD64 AddrBase);

typedef DWORD64 (__stdcall *getModuleBase_t)(IN HANDLE hProcess, IN DWORD64 dwAddr);

typedef BOOL (__stdcall *initialize_t)(IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess);

typedef DWORD64 (__stdcall *loadModule_t)(IN HANDLE hProcess, IN HANDLE hFile,
	IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll);

static stackWalk_t stackWalk=NULL;
static initialize_t initialize=NULL;
static getLineFromAddr_t getLineFromAddr=NULL;
static getSymFromAddr_t getSymFromAddr=NULL;
static undecorateSymbolName_t undecorateSymbolName=NULL;
static functionTableAccess_t functionTableAccess=NULL;
static getModuleBase_t getModuleBase=NULL;
static loadModule_t loadModule=NULL;
static HMODULE debugHelp=NULL;
}

namespace tpie {
void backtrace(std::ostream & out, int depth){
	const size_t max_size=4096;
	
	HANDLE hThread = GetCurrentThread();
	HANDLE hProcess = GetCurrentProcess();
	DWORD pid=GetCurrentProcessId();

	if (debugHelp == NULL) {
		TCHAR  buffer[max_size];
		if (GetEnvironmentVariable("ProgramFiles", buffer, max_size) > 0) {
			_tcscat_s(buffer, "\\Debugging Tools for Windows\\dbghelp.dll");
			debugHelp = LoadLibrary(buffer);
		}
		if (debugHelp == NULL && GetEnvironmentVariable("ProgramFiles", buffer, max_size) > 0) {
			_tcscat_s(buffer, "\\Debugging Tools for Windows 64-Bit\\dbghelp.dll");
			debugHelp = LoadLibrary(buffer);
		}
		debugHelp = LoadLibrary("dbghelp.dll");
		if (debugHelp == NULL) return;
	
		stackWalk = (stackWalk_t) GetProcAddress(debugHelp, "StackWalk64");
		if (stackWalk == NULL) return;
		getLineFromAddr = (getLineFromAddr_t) GetProcAddress(debugHelp, "SymGetLineFromAddr64");
		getSymFromAddr = (getSymFromAddr_t) GetProcAddress(debugHelp, "SymGetSymFromAddr64");
		undecorateSymbolName = (undecorateSymbolName_t) GetProcAddress(debugHelp, "UnDecorateSymbolName");
		functionTableAccess = (functionTableAccess_t) GetProcAddress(debugHelp, "SymFunctionTableAccess64");
		getModuleBase = (getModuleBase_t) GetProcAddress(debugHelp, "SymGetModuleBase64");
		initialize = (initialize_t) GetProcAddress(debugHelp, "SymInitialize");
		loadModule = (loadModule_t) GetProcAddress(debugHelp, "SymLoadModule64");

		if (functionTableAccess == NULL || getModuleBase == NULL || initialize == NULL) return;

		std::stringstream ss;
		ss << ".;";
		char temp[max_size];
		if (GetCurrentDirectoryA(max_size, temp) > 0) {
			temp[max_size-1] = '\0';
			ss << temp << ";";
		}
		if (GetModuleFileNameA(NULL, temp, max_size) > 0) {
			temp[max_size-1] = '\0';
			std::string ts(temp);
			ts.resize(ts.find_last_of("\\:/", ts.size()));
			ss << ts << ";";
		}
		if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", temp, max_size) > 0) {
			temp[max_size-1] = '\0';
			ss << temp << ";";
		}
		if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", temp, max_size) > 0) {
			temp[max_size-1] = '\0';
			ss << temp << ";";
		}
		if (GetEnvironmentVariableA("SYSTEMROOT", temp, max_size) > 0) {
			temp[max_size-1] = '\0';
			ss << temp << ";" << temp << "\\system32" << ";";
		}
		strncpy(temp, ss.str().c_str(), max_size);
		out << temp << std::endl;
		if (!initialize(hProcess, temp, false)) {
			TP_LOG_WARNING_ID("Could not initialize");
			return;
		}

		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(moduleEntry);

		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		for (BOOL ok=Module32First(h, &moduleEntry); !!ok; ok=Module32Next(h, &moduleEntry)) {
			if (!loadModule(hProcess, 0, moduleEntry.szExePath, moduleEntry.szModule,
				(DWORD64) moduleEntry.modBaseAddr, moduleEntry.modBaseSize)) {
				TP_LOG_WARNING_ID("Could not loadModule");
				return;
			}
		}
		CloseHandle(h);
	}

	CONTEXT c;
	memset(&c, 0, sizeof(CONTEXT)); 
	c.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&c);

	IMAGEHLP_SYMBOL64 *pSym = NULL;
  
    STACKFRAME64 s; // in/out stackframe
	memset(&s, 0, sizeof(s));
	DWORD imageType;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Mode = AddrModeFlat;
#ifdef _M_IX86
	// normally, call ImageNtHeader() and use machine info from PE header
	imageType = IMAGE_FILE_MACHINE_I386;
	s.AddrPC.Offset = c.Eip;
	s.AddrFrame.Offset = c.Ebp;
	s.AddrStack.Offset = c.Esp;
#elif _M_X64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	s.AddrPC.Offset = c.Rip;
	s.AddrFrame.Offset = c.Rsp;
	s.AddrStack.Offset = c.Rsp;
#elif _M_IA64
	imageType = IMAGE_FILE_MACHINE_IA64;
	s.AddrPC.Offset = c.StIIP;
	s.AddrFrame.Offset = c.IntSp;
	s.AddrBStore.Offset = c.RsBSP;
	s.AddrBStore.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.IntSp;
#else
#error "Platform not supported!"
#endif
	pSym = (IMAGEHLP_SYMBOL64 *) malloc(sizeof(IMAGEHLP_SYMBOL64) + max_size);
	memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + max_size);
	pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	pSym->MaxNameLength = max_size;

	IMAGEHLP_LINE64 lineInfo;
	memset(&lineInfo, 0, sizeof(lineInfo));
	lineInfo.SizeOfStruct = sizeof(lineInfo);

    DWORD64 offsetFromSymbol=0;
	DWORD offsetFromLine=0;

	backtrace_begin(out);
	for (int i=0; i < depth; ++i) {
		if (!stackWalk(imageType, hProcess, hThread, &s, &c, NULL, functionTableAccess, getModuleBase, NULL)) break;
		if (s.AddrPC.Offset == 0) break;
		char name[max_size];
		char file[max_size];
		name[0]='\0';
		file[0]='\0';
		int line=-1;
		if (getSymFromAddr && getSymFromAddr(hProcess, s.AddrPC.Offset, &offsetFromSymbol, pSym)) {
			if (!undecorateSymbolName(pSym->Name, name, max_size, UNDNAME_COMPLETE))
				strncpy(name, pSym->Name, 1024);
		}

		if (getLineFromAddr && getLineFromAddr(hProcess, s.AddrPC.Offset, &offsetFromLine, &lineInfo)) {
			line = lineInfo.LineNumber;
			strncpy(file, lineInfo.FileName, max_size);
		}
		backtrace_item(out, static_cast<size_t>(s.AddrPC.Offset), name, file, line);
	}
	backtrace_end(out);
}
}

#else // WIN32

// for __cxa_demangle
#include <cxxabi.h>

// for ::backtrace; see also GNU C lib docs 33.1
#include <execinfo.h>

#include <cstdlib>
#include <iostream>

namespace tpie {
void backtrace(std::ostream & out, int depth) {
	backtrace_begin(out);
	void * array[depth+1];
	int nSize = ::backtrace(array, depth+1);
	char ** symbols = backtrace_symbols(array, nSize);
	for(int i=1; i < nSize; ++i) {
		if (!symbols[i]) continue;
		std::string sym = symbols[i];
		
		std::string method, index;
		size_t r = sym.rfind(")");
		size_t m = sym.rfind("+",r);
		size_t l = sym.rfind("(",m);
		size_t s = sym.rfind("/", l);
		
		if (l == std::string::npos || 
			r == std::string::npos ||
			m == std::string::npos) {
			l = sym.rfind("[");
			method="Unknown";
			index="";
		} else {
			method=sym.substr(l+1,m-l-1);
			index=sym.substr(m,r-m);
		}
		if (s == std::string::npos) s=0;
		std::string exe = sym.substr(s+1,l-s-1);
		{
			int x; 
			char * buff=abi::__cxa_demangle(method.c_str(), NULL, NULL, &x);
			if (x == 0) method = buff;
			std::free(buff);
		}
		out << exe << ": " << method << index << std::endl;
	}
	backtrace_end(out);
	std::free(symbols);
}
}
#endif //WIN32

namespace tpie {
void __softassert(const char * expr, const char * file, int line) {
	log_error() << "Soft assertion error: " << expr << std::endl
				<< file << ":" << line << std::endl;
	backtrace(log_error());
}


}


