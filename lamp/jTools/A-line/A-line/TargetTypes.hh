/*	==============
 *	TargetTypes.hh
 *	==============
 */

#ifndef ALINE_TARGET_TYPES_HH
#define ALINE_TARGET_TYPES_HH


namespace tool
{
	
	enum ProductType
	{
		productNotBuilt = 0,
		productSource,
		productApplication,
		productStaticLib,
		productSharedLib,
		productTool,
		productToolkit,
		productINIT,
		productDriver
	};
	
	inline bool ProductGetsBuilt( ProductType product )
	{
		return product > productSource;
	}
	
	enum Toolchain
	{
		toolchainUnspecified = 0,
		toolchainMetrowerks,
		toolchainGNU
	};
	
	enum EnvType
	{
		envUnspecified = 0,
		envLamp,
		envUnix
	};
	
	enum BuildVariety
	{
		buildDefault,
		buildDebug,
		buildRelease,
		buildDemo
	};
	
}

#endif

