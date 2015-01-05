/*
	filehandle_method_set.hh
	------------------------
*/

#ifndef VFS_METHODS_FILEHANDLEMETHODSET_HH
#define VFS_METHODS_FILEHANDLEMETHODSET_HH


namespace vfs
{
	
	struct bstore_method_set;
	struct socket_method_set;
	struct stream_method_set;
	struct general_method_set;
	struct terminal_method_set;
	
	
	struct filehandle_method_set
	{
		const bstore_method_set*    bstore_methods;
		const socket_method_set*    socket_methods;
		const stream_method_set*    stream_methods;
		const general_method_set*   general_methods;
		const terminal_method_set*  terminal_methods;
	};
	
}

#endif
