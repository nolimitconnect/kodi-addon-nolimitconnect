//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once
//! implement singleton
template<typename T> class VxSingleton 
{
public:
	static T &Instance() 
	{
		static T VxSingletonInstance;
		return VxSingletonInstance;
	}
private:
	//=== constructor ===//
	VxSingleton();
	//=== constructor ===//
	VxSingleton(VxSingleton const &Object);
	//=== destructor ===//
	~VxSingleton();
	//=== operator ===//
	VxSingleton &operator=(VxSingleton const &Object);
};
