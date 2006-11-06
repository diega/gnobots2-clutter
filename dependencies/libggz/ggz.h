/**
 * @file   ggz.h
 * @author Brent M. Hendricks
 * @date   Fri Nov  2 23:32:17 2001
 * $Id$
 * 
 * Header file for ggz components lib
 *
 * Copyright (C) 2001 Brent Hendricks.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef __GGZ_H__
#define __GGZ_H__

#define LIBGGZ_VERSION_MAJOR 0
#define LIBGGZ_VERSION_MINOR 0
#define LIBGGZ_VERSION_MICRO 14
#define LIBGGZ_VERSION_IFACE "4:0:2"

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Allow easy use of GCC's "attribute" macro for debugging.
 *
 * Under gcc, we use the __attribute__ macro to check variadic arguments,
 * for instance to printf-style functions.  Other compilers may be able
 * to do something similar, but this is generally unnecessary since it's
 * only realy purpose is to give warning messages when the developer compiles
 * the code.
 */
#ifdef __GNUC__
#  define ggz__attribute(att)  __attribute__(att)
#else
#  define ggz__attribute(att)
#endif

#if defined __GNUC__ && (__GNUC__ >= 3)
#  define _GGZFUNCTION_ __FUNCTION_
#elif defined(__sun)
#  define _GGZFUNCTION_ ""
#else
#  ifndef __cplusplus
#    define _GGZFUNCTION_ __FUNCTION__
#  else
#    define _GGZFUNCTION_ ""
#  endif
#endif

/**
 * @defgroup memory Memory Handling
 *
 * These macros provide an alternative to the normal C library
 * functions for dynamically allocating memory.  They keep track of
 * memory allocated by storing the name of the function and file in
 * which they were called.
 *
 * You can then call ggz_memory_check() to make sure all allocated
 * memory has been freed.  Note that you will need to enable MEMORY
 * debugging to see this.
 * 
 * @{
 */

/** @brief Debugging type for memory debugging.
 *  @see ggz_debug_enable
 */
#define GGZ_MEM_DEBUG "ggz_mem"

/** 
 * Macro for memory allocation
 * 
 * @param size the size of memory to allocate, in bytes
 * 
 * @return a pointer to the newly allocated and zeroed memory
 */
#define ggz_malloc(size) _ggz_malloc(size, _GGZFUNCTION_ " in " __FILE__, __LINE__)
						   

/** 
 * Macro for resizing previously allocated memory
 * 
 * @param mem pointer to memory to reallocate
 * @param size new size requested, in bytes
 * 
 * @return pointer to allocated memory
 */
#define ggz_realloc(mem, size) _ggz_realloc(mem, size, _GGZFUNCTION_ " in " __FILE__, __LINE__)
						       

/** 
 * Macro for freeing memory previously allocated 
 * 
 * @param mem pointer to allocated memory
 * 
 * @return failure code
 */
#define ggz_free(mem) _ggz_free(mem, _GGZFUNCTION_ " in " __FILE__,  __LINE__)
						 

/** 
 * Macro for duplicating string
 * 
 * @param string string to duplicate
 * 
 * @return pointer to new string
 *
 * @note It is safe to pass a NULL string.
 */
#define ggz_strdup(string) _ggz_strdup(string, _GGZFUNCTION_ " in " __FILE__,  __LINE__)
						 


/** 
 * Function to actually perform memory allocation.  Don't call this
 * directly.  Instead, call ggz_malloc().
 * 
 * @param size size of memory to allocate, in bytes
 * @param tag string describing the calling function
 * @param line linenumber
 * 
 * @return pointer to newly allocated, zeroed memory
*/
void * _ggz_malloc(const size_t size, const char * tag, int line);

/** 
 * Function to perform memory reallocation.  Don't call this
 * directly.  Instead, call ggz_realloc().
 * 
 * @param ptr pointer to memory to reallocate
 * @param size new size, in bytes
 * @param tag string describing the calling function
 * @param line linenumber
 * 
 * @return pointer to allocated memory
*/
void * _ggz_realloc(const void * ptr, const size_t size,
                    const char * tag, int line);

/** 
 * Function to free allocated memory.  Don't call this
 * directly.  Instead, call ggz_free().
 * 
 * @param ptr pointer to memory
 * @param tag string describing the calling function
 * @param line linenumber
 * 
 * @return 0 on success, -1 on error
*/
int _ggz_free(const void * ptr, const char * tag, int line);

/** 
 * Function to copy a string.  Don't call this
 * directly.  Instead, call ggz_strdup().
 * 
 * @param ptr string to duplicate
 * @param tag string describing the calling function
 * @param line linenumber
 * 
 * @return newly allocated string
 *
 * @note It is safe to pass a NULL string.
 */
char * _ggz_strdup(const char * ptr, const char * tag, int line);


/** 
 * Check memory allocated against memory freed and display any discrepancies.
 * 
 * 
 * @return 0 if no allocated memory remains, -1 otherwise
 */
int ggz_memory_check(void);

/** @} */


/**
 * @defgroup conf Configuration file parsing
 *
 * Configuration file routines to store and retrieve values.
 * Configuration file parsing begins by calling ggz_conf_parse() to open
 * a config file.  The file can be created automatically if GGZ_CONF_CREATE
 * is specified.  The returned handle uniquely identifies the configuration
 * file, so multiple files can be open at one time.
 *
 * If the same configuration file is opened multiple times, the original
 * handle will be returned and only one copy will be retained within memory.
 * One use of this feature is that a file may be opened read only initially and
 * later have writing enabled by merely reparsing the file using the same
 * pathname.  Note that this feature can be fooled if the same file is
 * opened using different pathnames.  Chaos may or may not result in this
 * case, and it is considered a feature not a bug.
 *
 * Values are stored using a system of section and keys.  A key must be
 * unique within a section (you cannot store both an integer and a string
 * under the same key.  Section and key names may contain any characters
 * (including whitespace) other than an equals sign.  Although keys may not
 * have leading or trailing whitespace, section names may.  It is suggested
 * that any whitespace (other than possibly internal spaces) be avoided when
 * specifying section and key names.  Alphabetic case is preserved, and must
 * be matched.
 *
 * An important caveat to remember when using the configuration functions
 * is that writing a value only caches the value in memory.  In order to
 * write the values to the physical file, ggz_conf_commit() must be called
 * at some point.  This makes writing multiple values in rapid succession
 * more efficient, as the entire file must be regenerated in order to be
 * written to the flat-file format of the configuration file.
 *
 * The string and list reading functions return dynamically allocated
 * memory to the caller.  The user is responsible for calling ggz_free() on
 * this memory when they no longer need the returned values.
 *
 * All memory used internally by the configuration functions will be
 * released when ggz_conf_cleanup() is called.  Note that this does NOT
 * commit any changes made to the configuration files.
 * The user is expected to call this at program termination, but it may
 * be called at any time earlier than termination and new files may
 * be subsequently opened.
 *
 * @bug The maximum length of a configuration file line is fixed at 1024
 * characters.  Exceeding this length will result in parsing errors when
 * the file is parsed.  No internal problems should (hopefully) result,
 * but values will be lost.
 * @{
 */

/** @brief Debugging type for config-file debugging.
 *  @see ggz_debug_enable
 */
#define GGZ_CONF_DEBUG "ggz_conf"

/** 
 * Specifies the mode for opening a configuration file.
 * @see ggz_conf_parse()
 */
typedef enum {
	GGZ_CONF_RDONLY = ((unsigned char) 0x01), /**< Read only */
	GGZ_CONF_RDWR = ((unsigned char) 0x02),   /**< Read and write */
	GGZ_CONF_CREATE = ((unsigned char) 0x04)  /**< Create file */
} GGZConfType;


/**
 * Closes all open configuration files.
 * @note This does not automatically commit changed values.  Any non-committed
 * values written will be lost.
 */
void ggz_conf_cleanup (void);

/**
 * Closes one configuration file.
 * @note The same warning as for ggz_conf_cleanup() applies here.
 */
void ggz_conf_close (int handle);

/**
 * Opens a configuration file and parses the variables so they can
 * be retrieved with the access functions.
 * @param path A string specifying the filename to be parsed
 * @param options An or'ed set of GGZConfType option bits
 * @return An integer configuration file handle or -1 on error
 * @see GGZConfType
 */
int ggz_conf_parse		(const char *path,
				 const GGZConfType options);

/**
 * Commits any changed variables to the configuration file. The configuration
 * file remains open and may continue to be written to.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @return 0 if successful, -1 on failure
 */
int ggz_conf_commit		(int handle);

/**
 * Writes a string value to a section and key in an open configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to write to
 * @param key A string variable key name to write to
 * @param value The string value to write
 * @return 0 if successful, -1 on failure
 */
int ggz_conf_write_string	(int	handle,
			 const char	*section,
			 const char	*key,
			 const char	*value);

/**
 * Writes an integer value to a section and key in an open configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to write to
 * @param key A string variable key name to write to
 * @param value The integer value to write
 * @return 0 if successful, -1 on failure
 */
int ggz_conf_write_int	(int	handle,
			 const char	*section,
			 const char	*key,
			 int	value);

/**
 * Writes a list of string values to a section and key in an open
 * configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to write to
 * @param key A string variable key name to write to
 * @param argc The number of string array entries in argv
 * @param argv An array of strings to create a list from
 * @return 0 if successful, -1 on failure
 */
int ggz_conf_write_list	(int	handle,
			 const char	*section,
			 const char	*key,
			 int	argc,
			 char	**argv);

/**
 * Reads a string value from an open configuration file.  If the section/key
 * combination is not found, the default entry is returned.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to read from
 * @param key A string variable key name to read from
 * @param def A value to be returned if the entry does not exist (may be NULL)
 * @return A dynamically allocated copy of the stored (or default) value
 * or NULL
 * @note The copy is allocated via a standard ggz_malloc() call and the
 * caller is expected to be responsible for calling ggz_free() on the
 * returned value when they no longer need the value.  No memory is allocated
 * if a default value of NULL is returned.
 */
char * ggz_conf_read_string	(int	handle,
			 const char	*section,
			 const char	*key,
			 const char	*def);

/**
 * Reads an integer value from an open configuration file.  If the section/key
 * combination is not found, the default entry is returned.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to read from
 * @param key A string variable key name to read from
 * @param def A value to be returned if the entry does not exist
 * @return The integer value stored in the configuration file, or the default
 */
int ggz_conf_read_int	(int	handle,
			 const char	*section,
			 const char	*key,
			 int	def);

/**
 * Reads a list of string values from an open configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to read from
 * @param key A string variable key name to read from
 * @param argcp A pointer to an integer which will receive the number of
 * list entries read
 * @param argvp A pointer to a string array.  This will receive a value
 * pointing to a dynamically allocated array of string values.  The
 * list will be NULL-terminated.
 * @return 0 on success, -1 on failure
 * @note The array is allocated via standard ggz_malloc() calls and the
 * caller is expected to be responsible for calling ggz_free() on the string
 * values and the associated array structure when they no longer need the
 * list.  If the section/key combination is not found -1 will be returned,
 * *argcp is set to a value of zero, no memory will be allocated, and
 * *argvp will be set to NULL.
 */
int ggz_conf_read_list(int	handle,
			 const char	*section,
			 const char	*key,
			 int	*argcp,
			 char	***argvp);

/**
 * This will remove an entire section and all its associated keys from
 * a configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name to remove
 * @return 0 on success, -1 on failure
 */
int ggz_conf_remove_section	(int	handle,
			 const char	*section);

/**
 * This will remove a single key from a configuration file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name the key is located in
 * @param key A string key name to remove
 * @return 0 on success, -1 on failure
 */
int ggz_conf_remove_key	(int	handle,
			 const char	*section,
			 const char	*key);

/**
 * This function returns a list of all sections in a config file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param argcp A pointer to an integer which will receive the number of
 * sections in the configuration file
 * @param argvp A pointer to a string array.  This will receive a value
 * pointing to a dynamically allocated array of string values.  This list
 * is NOT NULL terminated.
 * @return 0 on success, -1 on failure
 * @note The array is allocated via standard ggz_malloc() calls and the
 * caller is expected to be responsible for calling ggz_free() on the string
 * values and the associated array structure when they no longer need the
 * list. 
 */
int ggz_conf_get_sections (int handle,
			 int	*argcp,
			 char	***argvp);

/**
 * This function returns a list of all keys within a section in a config file.
 * @param handle A valid handle returned by ggz_conf_parse()
 * @param section A string section name
 * @param argcp A pointer to an integer which will receive the number of
 * lists within section in the configuration file
 * @param argvp A pointer to a string array.  This will receive a value
 * pointing to a dynamically allocated array of string values.  This list
 * is NOT NULL terminated.
 * @return 0 on success, -1 on failure
 * @note The array is allocated via standard ggz_malloc() calls and the
 * caller is expected to be responsible for calling ggz_free() on the string
 * values and the associated array structure when they no longer need the
 * list. 
 */
int ggz_conf_get_keys (int handle,
			 const char *section,
			 int	*argcp,
			 char	***argvp);

/** @} */


/**
 * @defgroup list List functions 
 *
 * Data Structures and functions for manipulating linked-lists.
 * @{
 */

/** @brief A function type for doing data comparison on two items in a
 *  ::GGZList.
 *
 *  @param a An arbitrary element in the ::GGZList.
 *  @param b An arbitrary element in the ::GGZList.
 *  @return Negative if a < b; 0 if a == b; positive if a > b.  */
typedef int (*ggzEntryCompare)(const void *a, const void *b);


/** @brief A function type for creating a copy of a data item for
 *  insertion into a ::GGZList.
 *
 *  A function of this type may be called on an element when
 *  it is first inserted into a ::GGZList.
 *  @param data A pointer to the data given to the list for insertion
 *  @return A new copy of the data, safe for list insertion
 */
typedef	void *	(*ggzEntryCreate)	(void *data);


/** @brief A function type to destroy an entry in a ::GGZList.
 *
 *  A function of this type may be called on an element when
 *  it is removed from a ::GGZList.
 *  @param data The entry being removed
 */
typedef	void	(*ggzEntryDestroy)	(void *data);


/** @brief A single entry in a ::GGZList.
 *
 *  Do not access these members directly, but use the
 *  ggz_list_get_data(), ggz_list_next() and ggz_list_prev()
 *  accessor functions instead.
 */
typedef struct _GGZListEntry {
	void		     *data;     /**< Pointer to data for this node */
	struct _GGZListEntry *next, /**< Pointer to next nodes in the list */
		             *prev;     /**< Pointer to previous node in the list */
} GGZListEntry;


/** @brief Simple doubly-linked list.
 *
 *  Do not access these members directly.  Instead use accessor functions.
 */
typedef struct _GGZList {
	GGZListEntry	*head,         /**< Pointer to the first node in the list */
		        *tail;         /**< Pointer to the last node in the list */
	ggzEntryCompare	compare_func;  /**< Function used to compare data entries */
	ggzEntryCreate	create_func;   /**< Function used to copy data entries */
	ggzEntryDestroy	destroy_func;  /**< Function used to destroy data entries */
	int		options;       /**< List options */
	int		entries;       /**< The current number of list entries (ie. list length) */
} GGZList;



#define GGZ_LIST_REPLACE_DUPS 0x00 /**< Overwrite duplicate values on insert */
#define GGZ_LIST_ALLOW_DUPS   0x01 /**< Allow duplicate data entries to exist in the list */

/** @brief Create a new ::GGZList.
 *
 * When creating a a new list, you have some control over its
 * behavior.  The first parameter, compare_func allows you to specify
 * a comparison for sorting the list elements.  If you specify NULL
 * for compare_func, the list will be unordered.  The second
 * parameter, create_func allows you to specify how new copies of data
 * will be created during insertion.  ::GGZList objects stores
 * pointers to your data in ::GGZListEntry nodes. If you specify NULL
 * for create_func, the list will store the actual pointer to your
 * data when you call ggz_list_insert().  If you specify a
 * create_func, it will be called to create a new copy of the object
 * for storage in the list.  You are then safely deallocate the
 * original copy of the data.  The third parameter, destroy_func
 * allows you to specify a deallocation function for data entries when
 * they are removed from the list.
 * @note The functions ggz_list_compare_str(), ggz_list_create_str(),
 * ggz_list_destroy_str() are provided for use with character string
 * data.
 *
 * The last argument must be one of #GGZ_LIST_REPLACE_DUPS or
 * #GGZ_LIST_ALLOW_DUPS, to specify how the list will behave with
 * respect to duplicate data entries.  If #GGZ_LIST_REPLACE_DUPS is
 * passed, duplicate entries (as determined by compare_func) will be
 * replaced upon ggz_list_insert().  If #GGZ_LIST_ALLOW_DUPS is
 * specified, duplicate data entries will be allowed to exist in the
 * list.
 *
 * @note If compare_func is NULL, #GGZ_LIST_REPLACE_DUPS has no meaning.
 *
 * @param compare_func Function to use for comparing data items
 * @param create_func Function to use for allocating and copying data items
 * @param destroy_func Function to use for dellocating data items
 * @param options One of #GGZ_LIST_REPLACE_DUPS or #GGZ_LIST_ALLOW_DUPS
 * specifying how the list should handle duplicate data entries
 * @return A pointer to a newly allocated ::GGZList
 */
GGZList *ggz_list_create (ggzEntryCompare compare_func,
			  ggzEntryCreate create_func,
			  ggzEntryDestroy destroy_func,
			  int options);


/** @brief Insert data into a list.
 *
 * @param list Pointer to a ::GGZList
 * @param data Pointer to data to be inserted
 * @return -1 on failure, 0 is the item was inserted, and 1 if the
 * item replaced an existing list item 
 * @note Replacement of duplicate items only occurs if #GGZ_LIST_REPLACE_DUPS
 * was passed to ggz_list_create().
 */
int ggz_list_insert            (GGZList *list, void *data);


/** @brief Get the first node of a list.
 *
 * @param list Pointer to a ::GGZList
 * @return The ::GGZListEntry that is first in the list
 */
GGZListEntry *ggz_list_head	(GGZList *list);


/** @brief Get the last node of a list.
 *
 * @param list Pointer to a ::GGZList
 * @return The ::GGZListEntry that is last in the list
 */
GGZListEntry *ggz_list_tail	(GGZList *list);


/** @brief Get the next node of a list.
 *
 * @param entry Pointer to a ::GGZListEntry in a ::GGZList
 * @return The next ::GGZListEntry in the list
 */
GGZListEntry *ggz_list_next  	(GGZListEntry *entry);


/** @brief Get the previous node of a list.
 *
 * @param entry Pointer to a ::GGZListEntry in a ::GGZList
 * @return The previous ::GGZListEntry in the list
 */
GGZListEntry *ggz_list_prev	(GGZListEntry *entry);


/** @brief Retrieve the data stored in a list entry.
 *
 * @param entry Pointer to a ::GGZListEntry
 * @return Pointer to the data stored in the specifed node (::GGZListEntry)
 */
void *ggz_list_get_data		(GGZListEntry *entry);


/** @brief Search for a specified data item in the list.
 *
 * ggz_list_search() searches a list for a particular data item using
 * the ggzEntryCompare function provided as compare_func to
 * ggz_list_create().  If you wish to search using an alternative
 * comparison function, see ggz_list_search_alt().
 *
 * @param list Pointer to a ::GGZList
 * @param data Pointer to data to search for
 * @return Pointer to the ::GGZListEntry containing the specifed node
 * (NULL if the data could not be found or if no compare_func was
 * specified at list-creation time) 
 */
GGZListEntry *ggz_list_search	(GGZList *list, void *data);


/** @brief Search for a specified data item in the list using a provided comparison function.
 *
 * ggz_list_search_alt() searches a list for a particular data item
 * using the passed ggzEntryCompare function.
 *
 * @param list Pointer to a ::GGZList
 * @param data Pointer to data to search for
 * @param compare_func Comparison function
 * @return Pointer to the ::GGZListEntry containing the specified node
 * (NULL if the data could not be found or if no compare_func was
 * specified) 
 */
GGZListEntry *ggz_list_search_alt(GGZList *list, void *data,
				  ggzEntryCompare compare_func);

/** @brief Removes an entry from a list, calling a destructor if registered.
 *
 * ggz_list_delete_entry() removes the specifed entry from the list.
 * If a ggzEntryDestroy function was passed as destroy_func to
 * ggz_list_create(), it will be called on the data item after it has
 * been removed.
 *
 * @param list Pointer to a ::GGZList
 * @param entry Pointer to the ::GGZListEntry to remove
 */
void ggz_list_delete_entry	(GGZList *list, GGZListEntry *entry);


/** @brief Free all resources associated with a list.
 *
 * ggz_list_free() will free all resources allocated by the list.  If
 * a ggzEntryDestroy function was passed as destroy_func to
 * ggz_list_create(), it will be called on all data items in the list
 * as well.
 *
 * @param list Pointer to a ::GGZList
 */
void ggz_list_free		(GGZList *list);


/** @brief Get the length of the list.
 *
 * @param list Pointer to a ::GGZList
 * @return The number of entries in the list
 */
int ggz_list_count		(GGZList *list);


/* String list functions */

/** @brief Compare two character strings.
 *
 *  This function is intended to be passed as the compare_func of
 *  ggz_list_create() when creating a list that stores character
 *  strings.
 *
 * @param a A string to compare 
 * @param b A second string to compare 
 * @return The result of strcmp() on a and b, or 1 if either is NULL
 * */
int ggz_list_compare_str	(void *a, void *b);


/** @brief Copy a character string.
 *
 *  This function is intended to be passed as the create_func of
 *  ggz_list_create() when creating a list that stores character
 *  strings.
 *
 *  @param data A string to copy
 *  @return A newly allocated copy of the string
 */
void * ggz_list_create_str	(void *data);


/** @brief Free a character string.
 *
 *  This function is intended to be passed as the destroy_func of
 *  ggz_list_create() when creating a list that stores character
 *  strings.
 *
 *  @param data The string to deallocate
 */
void ggz_list_destroy_str	(void *data);

/** @} */


/**
 * @defgroup stack Stacks
 *
 * Data Structures and functions for manipulating stacks.
 *
 * @{
 */

/** @brief Simple implementation of stacks using ::GGZList.
 */
typedef struct _GGZList GGZStack;


/** @brief Create a new stack.
 *
 * @return Pointer to a newly allocated ::GGZStack object
 */
GGZStack* ggz_stack_new(void);


/** @brief Push a data item onto the top of the stack.
 *
 * @param stack Pointer to a ::GGZStack
 * @param data Pointer to data to insert onto stack
 */
void ggz_stack_push(GGZStack *stack, void *data);


/** @brief Pop the top item off of the stack.
 *
 * @param stack Pointer to a ::GGZStack
 * @return Pointer to the data item on the top of the stack
 */
void* ggz_stack_pop(GGZStack *stack);


/** @brief Get the top item on the stack without popping it.
 *
 * @param stack Pointer to a ::GGZList
 * @return Pointer to the data item currently in to of the stack
 */
void* ggz_stack_top(GGZStack *stack);


/** @brief Free the stack.
 *
 * @note This does not free the data stored in the stack.
 *
 * @param stack Pointer to a ::GGZStack
 */
void ggz_stack_free(GGZStack *stack);

/** @} */


/**
 * @defgroup xml XML parsing 
 *
 * Utility functions for doing simple XML parsing.  These can be used
 * with streaming XML parsers, and don't have the overhead of a full
 * DOM tree.  ::GGZXMLElement represents a single element, along with
 * its attributes and text data.  
 *
 * @note This does not parse your XML.  It is simply for use to store
 * the data as you are parsing.
 *
 * @{ */

/** @brief Object representing a single XML element.
 *
 *  Except for "process", do not access these members directly.
 *  Instead use the provided accessor functions.  "process" is meant
 *  to be invoked as a method on instances of GGZXMLElement.
 */
struct _GGZXMLElement {
	
	char *tag;           /**< The name of the element */
	char *text;          /**< Text content of an element */
	GGZList *attributes; /**< List of attributes on the element */
	void *data;          /**< Extra data associated with tag (usually gleaned from children) */
	void (*free)(struct _GGZXMLElement*); /**< Function to free allocated memory */
	void (*process)(void*, struct _GGZXMLElement*); /**< Function to "process" tag */
};


/** @brief Object representing a single XML element.
 */
typedef struct _GGZXMLElement GGZXMLElement;


/** @brief Create a new ::GGZXMLElement element.
 *
 * @param tag The name of the XML element (tag)
 * @param attrs NULL terminated array of attributes/values.  These must alternate: attribute1, value1, attribute2, value2, etc.
 * @param process User-defined function for processing XML elements
 * @param free User-defined function for deallocating ::GGZXMLElement
 * objects.  If provided, this will be invoked by
 * ggz_xmlelement_free(), and in addition to any user-defined
 * processing should call ggz_free() the element itself.
 * @return Pointer to a newly allocated ::GGZXMLElement object 
 */
GGZXMLElement* ggz_xmlelement_new(const char *tag, const char * const *attrs,
	void (*process)(void*, GGZXMLElement*), void (*free)(GGZXMLElement*));


/** @brief Initialize a ::GGZXMLElement.
 *
 * @param element Pointer to a ::GGZXMLElement to initialize
 * @param tag The name of the XML element (tag)
 * @param attrs NULL terminated array of attributes/values.  These must alternate: attribute1, value1, attribute2, value2, etc.
 * @param process User-defined function for processing XML elements
 * @param free User-defined function for deallocating ::GGZXMLElement
 * objects.  If provided, this will be invoked by
 * ggz_xmlelement_free(), and in addition to any user-defined
 * processing should call ggz_free() the element itself.
 * @return Pointer to a newly allocated ::GGZXMLElement object 
 */
void ggz_xmlelement_init(GGZXMLElement *element, const char *tag,
	const char * const *attrs,
	void (*process)(void*, GGZXMLElement*), void (*free)(GGZXMLElement*));


/** @brief Set ancillary data on a ::GGZXMLElement object.
 *
 * Associate some extra data with an XML element.
 * 
 * @param element Pointer to an XML element
 * @param data Pointer to user-supplied data
 * @return The element's name
 */
void ggz_xmlelement_set_data(GGZXMLElement *element, void *data);


/** @brief Get an XML element's name.
 *
 * @param element Pointer to an XML element
 * @return The element's name
 */
const char *ggz_xmlelement_get_tag(GGZXMLElement *element);


/** @brief Get the value of an attribute on XML element.
 *
 * @param element Pointer to an XML element
 * @param attr An attribute name
 * @return The value of the attribute, or NULL is there is no such
 * attribute present 
 */
const char *ggz_xmlelement_get_attr(GGZXMLElement *element, const char *attr);


/** @brief Get the user-supplied data associated with an XML element.
 *
 * @param element Pointer to an XML element
 * @return Pointer to the user-supplied data
 */
void* ggz_xmlelement_get_data(GGZXMLElement *element);


/** @brief Get an XML element's content text.
 *
 * @param element Pointer to an XML element
 * @return The text content of the element
 */
char* ggz_xmlelement_get_text(GGZXMLElement *element);


/** @brief Append a string to the element's content text.
 *
 * @param element Pointer to an XML element
 * @param text String to append
 * @param len The string's length, in bytes
 */
void ggz_xmlelement_add_text(GGZXMLElement *element, const char *text, int len);


/** @brief Free the memory associated with an XML element.
 *
 * @param element Pointer to an XML element
 */
void ggz_xmlelement_free(GGZXMLElement *element);

/** @} */


/**
 * @defgroup debug Debug/error logging
 * 
 * Functions for debugging and error messages.
 * @{
 */

/** @brief What memory checks should we do?
 *
 *  @see ggz_debug_cleanup
 */
typedef enum {
	GGZ_CHECK_NONE = 0x00, /**< No checks. */
	GGZ_CHECK_MEM = 0x01   /**< Memory (leak) checks. */
} GGZCheckType;

/** @brief A callback function to handle debugging output.
 *
 *  A function of this type can be registered as a callback handler to handle
 *  debugging output, rather than having the output go to stderr or to a file.
 *  If this is done, each line of output will be sent directly to this function
 *  (no trailing newline will be appended).
 *
 *  @see ggz_debug_set_func
 *  @param priority The priority of the log, i.e. LOG_DEBUG; see syslog()
 *  @param msg The debugging output message
 *  @note If your program is threaded, this function must be threadsafe.
 */
typedef void (*GGZDebugHandlerFunc)(int priority, const char *msg);

/**
 * @brief Initialize and configure debugging for the program.
 * 
 * This should be called early in the program to set up the debugging routines.
 * @param types A null-terminated list of arbitrary string debugging "types"
 * @param file A file to write debugging output to, or NULL for none
 * @see ggz_debug
 */
void ggz_debug_init(const char **types, const char* file);

/** @brief Set the debug handler function.
 *
 *  Call this function to register a debug handler function.  NULL can
 *  be passed to disable the debug handler.  If set, the debug handler
 *  function will be called to handle debugging output, overriding any
 *  file that had previously been specified.
 *
 *  @param func The new debug handler function
 *  @return The previous debug handler function
 *  @note This function is not threadsafe (re-entrant).
 */
GGZDebugHandlerFunc ggz_debug_set_func(GGZDebugHandlerFunc func);

/**
 * @brief Enable a specific type of debugging.
 *
 * Any ggz_debug() calls that use that type will then be logged.
 * @param type The "type" of debugging to enable
 * @see ggz_debug
 */
void ggz_debug_enable(const char *type);

/**
 * @brief Disable a specific type of debugging.
 *
 * Any ggz_debug() calls that use the given type of debugging will then not be
 * logged.
 * @param type The "type" of debugging to disable
 * @see ggz_debug
 */
void ggz_debug_disable(const char *type);

/**
 * @brief Log a debugging message.
 *
 * This function takes a debugging "type" as well as a printf-style list of
 * arguments.  It assembles the debugging message (printf-style) and logs it
 * if the given type of debugging is enabled.
 * @param type The "type" of debugging (similar to a loglevel)
 * @param fmt A printf-style format string
 * @see ggz_debug_enable, ggz_debug_disable
 */
void ggz_debug(const char *type, const char *fmt, ...)
               ggz__attribute((format(printf, 2, 3)));

/** @brief Log a notice message.
 *
 *  This function is nearly identical to ggz_debug(), except that if the
 *  debugging output ends up passed to the debug handler function,
 *  the priority will be LOG_NOTICE instead of LOG_DEBUG.  This is only
 *  of interest to a few programs.
 *  @param type The "type" of debugging (similar to a loglevel)
 *  @param fmt A printf-style format string
 *  @see ggz_debug, ggz_debug_set_func
 */
void ggz_log(const char *type, const char *fmt, ...)
             ggz__attribute((format(printf, 2, 3)));

/**
 * @brief Log a syscall error.
 *
 * This logs an error message in a similar manner to ggz_debug()'s debug
 * logging.  However, the logging is done regardless of whether
 * debugging is enabled or what debugging types are set.  errno and
 * strerror are also used to create a more useful message.
 * @param fmt A printf-style format string
 * @see ggz_debug
 */
void ggz_error_sys(const char *fmt, ...)
                   ggz__attribute((format(printf, 1, 2)));

/**
 * @brief Log a fatal syscall error.
 *
 * This logs an error message just like ggz_error_sys(), and also
 * exits the program.
 * @param fmt A printf-style format string
 */
void ggz_error_sys_exit(const char *fmt, ...)
                        ggz__attribute((format(printf, 1, 2)))
                        ggz__attribute((noreturn));

/**
 * @brief Log an error message.
 *
 * This logs an error message in a similar manner to ggz_debug()'s debug
 * logging.  However, the logging is done regardless of whether
 * debugging is enabled or what debugging types are set.
 * @param fmt A printf-style format string
 * @note This is equivalent to ggz_debug(NULL, ...) with debugging enabled.
 */
void ggz_error_msg(const char *fmt, ...)
                   ggz__attribute((format(printf, 1, 2)));

/**
 * @brief Log a fatal error message.
 *
 * This logs an error message just like ggz_error_msg(), and also
 * exits the program.
 * @param fmt A printf-style format string
 */
void ggz_error_msg_exit(const char *fmt, ...)
                        ggz__attribute((format(printf, 1, 2)))
                        ggz__attribute((noreturn));

/**
 * @brief Cleans up debugging state and prepares for exit.
 *
 * This function should be called right before the program exits.  It cleans
 * up all of the debugging state data, including writing out the memory check
 * data (if enabled) and closing the debugging file (if enabled).
 * @param check A mask of things to check
 */
void ggz_debug_cleanup(GGZCheckType check);

/** @} */


/**
 * @defgroup misc Miscellaneous convenience functions
 *
 * @{
 */

/**
 * Escape XML characters in a text string.
 * @param str The string to encode
 * @return A pointer to a dynamically allocated string with XML characters
 * replaced with ampersand tags, or NULL on error
 * @note The dynamic memory is allocated using ggz_malloc() and the caller is
 * expected to later free this memory using ggz_free().  If the original string
 * did not contain any characters which required escaping a ggz_strdup() copy
 * is returned.
 */
char * ggz_xml_escape(const char *str);

/**
 * Restore escaped XML characters into a text string.
 * @param str The string to decode
 * @return A pointer to a dynamically allocated string with XML ampersand tags
 * replaced with their normal ASCII characters, or NULL on error
 * @note The dyanmic memory is allocated using ggz_malloc() and the caller is
 * expected to later free this memory using ggz_free().  If the original string
 * did not contain any characters which required decoding, a ggz_strdup() copy
 * is returned.
 * @note When using expat, incoming text is automatically unescaped by the
 * expat library.  It is therefore generally not necessary to use this function
 * with expat.
 */
char * ggz_xml_unescape(const char *str);

/** @brief Structure used internally by ggz_read_line().
 */
typedef struct _GGZFile GGZFile;

/**
 * Setup a file structure to use with ggz_read_line().
 * @param fdes A preopened integer file descriptor to read from
 * @return A pointer to a dynamically allocated ::GGZFile structure
 * @note The user MUST have opened the requested file for reading before
 * using this function.  When finished using ggz_read_line(), the user should
 * cleanup this struct using ggz_free_file_struct().
 */
GGZFile * ggz_get_file_struct(int fdes);

/**
 * Create directories to fill out a path.  New directories are created with
 * permissions 700 (S_IRWXU).
 * @param full The full path to be created.
 * @return 0 on success; -1 on failure
 */
int ggz_make_path(const char *full);

/**
 * Read a line of arbitrary length from a file.
 * @param file A ::GGZFile structure allocated via ggz_get_file_struct()
 * @return A NULL terminated line from the file of arbitrary length or
 * NULL at end of file
 * @note The dynamic memory is allocated using ggz_malloc() and the caller is
 * expected to later free this memory using ggz_free().
 */
char * ggz_read_line(GGZFile *file);

/**
 * Deallocate a file structure allocated via ggz_get_file_struct().
 * @param file A ::GGZFile structure allocated via ggz_get_file_struct()
 * @note The caller is expected to close the I/O file before or after
 * freeing the file structure.
 */
void ggz_free_file_struct(GGZFile *file);


/**
 * String comparison function that is safe with NULLs.
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return An integer less than, equal to, or greater than zero if s1
 * is found, respectively, to be less than, to match, or be greater
 * than s2
 * @note NULL in considered to be less than any non-NULL string and
 * equal to itself
 */
int ggz_strcmp(const char *s1, const char *s2);


/** @brief Case-insensitive string comparison function that is safe with NULLs
 *  The function returns an integer less than, equal to, or greater than
 *  zero if s1 is found, respectively, to be less than, to match, or be greater
 *  than s2.  NULL in considered to be less than any non-NULL string
 *  and equal to itself
 *  @param s1 First string to compare
 *  @param s2 Second string to compare
 *  @return The comparison value.
 */
int ggz_strcasecmp(const char *s1, const char *s2);
 
/** @} */


/**
 * @defgroup easysock Easysock IO
 * 
 * Simple functions for reading/writing binary data across file descriptors.
 *
 * @{
 */

/** @brief ggz_debug debugging type for Easysock debugging.
 *
 *  To enable socket debugging, add this to the list of debugging types.
 *  @see ggz_debug_enable
 */
#define GGZ_SOCKET_DEBUG "socket"

/** @brief An error type for the GGZ socket functions.
 *
 *  If there is a GGZ socket error, the registered error handler will be
 *  called and will be given one of these error types.
 */
typedef enum {
	GGZ_IO_CREATE,  /**< Error creating a socket. */
	GGZ_IO_READ,    /**< Error reading from a socket. */
	GGZ_IO_WRITE,   /**< Error writing to a socket. */
	GGZ_IO_ALLOCATE /**< Error when the allocation limit is exceeded. */
} GGZIOType;

/** @brief A data type for the GGZ socket function error handler.
 *
 *  If there is a GGZ socket error, the registered error handler will be
 *  called and will be given one of these error data types.
 */
typedef enum {
	GGZ_DATA_NONE,   /**< No data is associated with the error. */
	GGZ_DATA_CHAR,   /**< The error occurred while dealing with a char. */
	GGZ_DATA_INT,    /**< The error occurred in dealing with an integer. */
	GGZ_DATA_STRING, /**< The error occurred in dealing with a string. */
	GGZ_DATA_FD      /**< Error while dealing with a file descriptor. */
} GGZDataType;

/** @brief An error function type.
 *
 *  This function type will be called when there is an error in a GGZ
 *  socket function.
 *
 *  @param msg The strerror message associated with the error
 *  @param type The type of error that occurred
 *  @param fd The socket on which the error occurred, or -1 if not applicable
 *  @param data Extra data associated with the error
 */
typedef void (*ggzIOError) (const char * msg,
                            const GGZIOType type,
                            const int fd,
			    const GGZDataType data);

/** @brief Set the ggz/easysock error handling function.
 *
 *  Any time an error occurs in a GGZ socket function, the registered error
 *  handling function will be called.  Use this function to register a new
 *  error function.  Any previous error function will be discarded.
 *
 *  @param func The new error-handling function
 *  @return 0
 *  @todo Shouldn't this function return a void or ggzIOError?
 */
int ggz_set_io_error_func(ggzIOError func);

/** @brief Remove the ggz/easysock error handling function.
 *
 *  The default behavior when a socket failure occurs in one of the GGZ socket
 *  functions is to do nothing (outside of the function's return value).  This
 *  may be overridden by registering an error handler with
 *  ggz_set_io_error_func(), but the behavior may be returned by calling this
 *  function to remove the error handler.
 *
 *  @return The previous error-handling function, or NULL if none.
 */
ggzIOError ggz_remove_io_error_func(void);


/** @brief An exit function type.
 *
 *  This function type will be called to exit the program.
 *  @param status The exit value
 */
typedef void (*ggzIOExit) (int status);

/** @brief Set the ggz/easysock exit function.
 *
 *  Any of the *_or_die() functions will call the set exit function if there
 *  is an error.  If there is no set function, exit() will be called.
 *  @param func The newly set exit function
 *  @return 0
 *  @todo Shouldn't this return a void?
 */
int ggz_set_io_exit_func(ggzIOExit func);

/** @brief Remove the ggz/easysock exit function.
 *
 *  This removes the existing exit function, if one is set.  exit() will
 *  then be called to exit the program.
 *  @return The old exit function (or NULL if none)
 */
ggzIOExit ggz_remove_io_exit_func(void);

/** @brief Get libggz's limit on memory allocation.
 *
 *  @return The limit to allow on ggz_read_*_alloc() calls, in bytes
 *  @see ggz_set_io_alloc_limit
 */
unsigned int ggz_get_io_alloc_limit(void);

/** @brief Set libggz's limit on memory allocation.
 *
 *  In functions of the form ggz_read_*_alloc(), libggz will itself
 *  allocate memory for the * object that is being read in.  This
 *  presents an obvious security concern, so we limit the amount of
 *  memory that can be allocated.  The default value is 32,767 bytes,
 *  but it can be changed by calling this function.
 *
 *  @param limit The new limit to allow on alloc-style calls, in bytes
 *  @return The previous limit
 *  @see ggz_get_io_alloc_limit
 *  @see ggz_read_string_alloc
 */
unsigned int ggz_set_io_alloc_limit(const unsigned int limit);


/** @brief Initialize the network.
 *
 *  This function will do anything necessary to initialize the network to
 *  use sockets (this is needed on some platforms).  It should be called at
 *  least once before any sockets are put to use.  Calling it more than once
 *  is harmless.  It will be called automatically at the start of all GGZ
 *  socket creation functions.
 *
 *  @return 0 on success, negative on failure
 */
int ggz_init_network(void);


/** @brief A network resolver function type.
 *
 *  This function type will be called whenever a hostname has been resolved
 *  or a socket has been created asynchronously.
 *  @param status The IP address of the host name
 *  @param socket File descriptor or error code like in ggz_make_socket
 */
typedef void (*ggzNetworkNotify) (const char *address, int socket);

/** @brief Set the ggz/easysock resolver notification function.
 *
 *  This function will be called whenever a resolving task submitted
 *  to ggz_resolvename() or ggz_make_socket() has finished.
 *  @param func The newly set resolver notification function
 *  @return 0
 *  @todo Shouldn't this return a void? (from ggz_set_io_exit_func)
 */
int ggz_set_network_notify_func(ggzNetworkNotify func);

/** @brief Resolve a host name.
 *
 *  In order to prevent blocking GUIs, this function can handle
 *  resolving a hostname into a numerical address asynchronously.
 *  The notification function will be called whenever it finishes.
 *  It receives as its argument the address, which might still be the
 *  same hostname in the case of errors. The result should be passed
 *  to gethostbyname() to receive the network data structures.
 *  If no notification function is set, this function returns the hostname
 *  as it is, without any lookup. If no GAI support is available, but a
 *  notification function is set, it is called with the unresolved hostname,
 *  too.
 *  @param name Hostname to resolve
 *  @return The hostname in case no notification function is set, or NULL
 *  @todo Should this resolve synchronously in the special cases above?
 */
const char *ggz_resolvename(const char *name);


/** @brief Get the IP address or host name of a connected peer.
 *
 *  This function tells about the IP address of the peer which is
 *  connected to the specified socket. If resolving is enabled, then
 *  the hostname is returned instead. In this case, resolving will
 *  be a blocking operation.
 *  @param fd Local end file descriptor of the connection
 *  @param resolve Whether or not to resolve the host name
 *  @return IP address or host name of peer, or NULL on error
 *  @note The string must be ggz_free()d afterwards
 */
const char *ggz_getpeername(int fd, int resolve);


/****************************************************************************
 * Creating a socket.
 * 
 * type  :  one of GGZ_SERVER or GGZ_CLIENT
 * port  :  tcp port number 
 * server:  hostname to connect to (only relevant for client)
 * 
 * Returns socket fd or -1 on error
 ***************************************************************************/

/** @brief A socket type.
 *
 *  These socket types are used by ggz_make_socket() and friends to decide
 *  what actions are necessary in making a connection.
 */
typedef enum {
	GGZ_SOCK_SERVER, /**< Just listen on a particular port. */
	GGZ_SOCK_CLIENT  /**< Connect to a particular port of a server. */
} GGZSockType;

/** @brief Make a socket connection.
 *
 *  This function makes a TCP socket connection.
 *    - For a server socket, we'll just listen on the given port and accept
 *      the first connection that is made there.
 *    - For a client socket, we'll connect to a server that is (hopefully)
 *      listening at the given port and hostname.
 *
 *  Note that when a ggzNetworkNotify callback has been set up, this function
 *  returns immediately and creates the socket later on.
 *
 *  @param type The type of socket (server or client)
 *  @param port The port to listen/connect to
 *  @param server The server hostname for clients, the interface address else
 *  @return File descriptor on success, -1 on creation error, -2 on lookup
 *  error, -3 when using asynchronous creation
 */
int ggz_make_socket(const GGZSockType type, 
		    const unsigned short port, 
		    const char *server);

/** @brief Make a socket connection, exiting on error.
 *
 *  Aside from the error condition, this is identical to ggz_make_socket().
 */
int ggz_make_socket_or_die(const GGZSockType type,
			   const unsigned short port, 
			   const char *server);

/** @brief Connect to a unix domain socket.
 *
 *  This function connects to a unix domain socket, a socket associated with
 *  a file on the VFS.
 *    - For a server socket, we unlink the socket file first and then
 *      create it.
 *    - For a client socket, we connect to a pre-existing socket file.
 *
 *  @param type The type of socket (server or client)
 *  @param name The name of the socket file
 *  @return The socket FD on success, -1 on error
 *  @note When possible, this should not be used.  Use socketpair() instead.
 */
int ggz_make_unix_socket(const GGZSockType type, const char* name);

/** @brief Connect to a unix domain socket, exiting on error.
 *
 *  Aside from the error condition, this is identical to ggz_make_unix_socket().
 */
int ggz_make_unix_socket_or_die(const GGZSockType type, const char* name);


/** @brief Write a character value to the given socket.
 *
 *  This function will write a single character to the socket.  The
 *  character will be readable at the other end with ggz_read_char.
 *
 *  @param sock The socket file descriptor to write to
 *  @param data A single character to write
 *  @return 0 on success, -1 on error
 */
int ggz_write_char(const int sock, const char data);

/** @brief Write a character value to the given socket, exiting on error.
 *
 *  @param sock The socket file descriptor to write to
 *  @param data A single character to write
 *  @note Aside from error handling, this is identical to ggz_write_char().
 */
void ggz_write_char_or_die(const int sock, const char data);

/** @brief Read a character value from the given socket.
 *
 *  This function will read a single character (as written by
 *  ggz_write_char()) from a socket.  It places the value into the
 *  character pointed to.
 *
 *  @param sock The socket file descriptor to read from
 *  @param data A pointer to a single character
 *  @return 0 on success, -1 on error
 */
int ggz_read_char(const int sock, char *data);

/** @brief Read a character value from the given socket, exiting on error.
 *
 *  @param sock The socket file descriptor to read from
 *  @param data A pointer to a single character
 *  @note Aside from error handling, this is identical to ggz_read_char().
 */
void ggz_read_char_or_die(const int sock, char *data);

/** @brief Write an integer to the socket in network byte order.
 *
 *  ggz_write_int() and ggz_read_int() can be used to send an integer across a
 *  socket.  The integer will be sent in network byte order, so the
 *  functions may safely be used across a network.  Note, though, that it
 *  is probably not safe to use this function to send structs or other
 *  data that may use a different representation than a simple integer.
 *
 *  @param sock The socket to write to
 *  @param data The integer to write
 *  @return 0 on success, -1 on error
 */
int ggz_write_int(const int sock, const int data);

/** @brief Write an integer to the socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_write_int().
 */
void ggz_write_int_or_die(const int sock, const int data);

/** @brief Read an integer from the socket in network byte order.
 *  
 *  @see ggz_write_int
 *
 *  @param sock The socket to read from
 *  @param data A pointer to an integer in which to place the data
 *  @return 0 on success, -1 on error
 */
int ggz_read_int(const int sock, int *data);

/** @brief Read an integer from the socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_read_int.
 */
void ggz_read_int_or_die(const int sock, int *data);

/** @brief Write a string to the given socket.
 *
 *  This function will write a full string to the given socket.  The string
 *  may be read at the other end by ggz_read_string() and friends.
 *
 *  @param sock The socket file descriptor to write to
 *  @param data A pointer to the string to write
 *  @return 0 on success, -1 on error
 */
int ggz_write_string(const int sock, const char *data);

/** @brief Write a string to the given socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_write_string().
 */
void ggz_write_string_or_die(const int sock, const char *data);

/** @brief Write a printf-style formatted string to the given socket.
 *
 *  This function allows a format string and a list of arguments
 *  to be passed in.  The function will assemble the string (printf-style)
 *  and write it to the socket.
 *
 *  @param sock The socket file descriptor to write to
 *  @param fmt A printf-style formatting string
 *  @param ... A printf-style list of arguments
 *  @note This function will write identically to ggz_write_string().
 */
int ggz_va_write_string(const int sock, const char *fmt, ...)
                        ggz__attribute((format(printf, 2, 3)));

/** @brief Write a formatted string to the socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_va_write_string.
 */
void ggz_va_write_string_or_die(const int sock, const char *fmt, ...)
                                ggz__attribute((format(printf, 2, 3)));

/** @brief Read a string from the given socket.
 *
 *  This function will read a full string from the given socket.  The string
 *  may be written at the other end by ggz_write_string() and friends.  The
 *  length of the string is given as well to avoid buffer overruns; any
 *  characters beyond this will be lost.
 *
 *  @param sock The socket file descriptor to read from
 *  @param data A pointer to the string to read; it will be changed
 *  @param len The length of the string pointed to by data
 *  @return 0 on success, -1 on error
 */
int ggz_read_string(const int sock, char *data, const unsigned int len);

/** @brief Read a string from the given socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_read_string().
 */
void ggz_read_string_or_die(const int sock, char *data, const unsigned int len);

/** @brief Read and allocate a string from the given socket.
 *
 *  This function reads a string from the socket, just like ggz_read_string().
 *  But instead of passing in a pre-allocated buffer to write in, here
 *  we pass a pointer to a string pointer:
 *
 *  @code
 *    char* str;
 *    if (ggz_read_string_alloc(fd, &str) >= 0) {
 *        // ... handle the string ...
 *        ggz_free(str);
 *    }
 *  @endcode
 *
 *  @param sock The socket file descriptor to read from
 *  @param data A pointer to an empty string pointer
 *  @return 0 on success, -1 on error
 *  @note The use of this function is a security risk.
 *  @see ggz_set_io_alloc_limit
 */
int ggz_read_string_alloc(const int sock, char **data);

/** @brief Read and allocate string from the given socket, exiting on error.
 *
 *  Aside from the error condition, this function is identical to
 *  ggz_read_string_alloc().
 */
void ggz_read_string_alloc_or_die(const int sock, char **data);

/* ggz_write_fd/ggz_read_fd are not supported on all systems. */

/** @brief Write a file descriptor to the given (local) socket.
 *
 *  ggz_write_fd() and ggz_read_fd() handle the rather tricky task of sending
 *  a file descriptor across a socket.  The FD is written with ggz_write_fd()
 *  and can be read at the other end by ggz_read_fd().  Note that this will
 *  only work for local sockets (i.e. not over the network).  Many thanks to
 *  Richard Stevens and his wonderful books, from which these functions come.
 *
 *  @param sock The socket to write to
 *  @param sendfd The FD to send across the socket
 *  @return 0 on success, -1 on error
 */
int ggz_write_fd(const int sock, int sendfd);

/** @brief Read a file descriptor from the given (local) socket.
 *  
 *  @see ggz_write_fd
 *
 *  @param sock The socket to read from
 *  @param recvfd The FD that is read
 *  @return 0 on success, -1 on error
 **/
int ggz_read_fd(const int sock, int *recvfd);

/** @brief Write a sequence of bytes to the socket.
 *
 *  ggz_writen() and ggz_readn() are used to send an arbitrary quantity of raw
 *  data across the a socket.  The data is written with ggz_writen() and can
 *  be read at the other end with ggz_readn().  Many thanks to Richard Stevens
 *  and his wonderful books, from which these functions come.
 *
 *  @param sock The socket to write to
 *  @param vdata A pointer to the data to write
 *  @param n The number of bytes of data to write from vdata
 *  @return 0 on success, -1 on error
 */
int ggz_writen(const int sock, const void *vdata, size_t n);

/** @brief Read a sequence of bytes from the socket.
 *
 *  @see ggz_writen
 *
 *  @param sock The socket to read from
 *  @param data A pointer a buffer of size >= n in which to place the data
 *  @param n The number of bytes to read
 *  @return 0 on success, -1 on error
 *  @note You must know how much data you want BEFORE calling this function.
 */
int ggz_readn(const int sock, void *data, size_t n);

/** @} */

/**
 * @defgroup security Security functions
 *
 * All functions related to encryption and encoding go here.
 *
 * Encryption functions use gcrypt, and will always fail if support for gcrypt
 * has not been compiled in. Encoding functions will always be available.
 *
 * @{
 */

/** @brief Hash data structure.
 *
 *  Contains a string and its length, so that NULL-safe
 *  functions are possible.
 */
typedef struct
{
	char *hash;		/**< Hash value */
	int hashlen;	/**< Length of the hash value, in bytes */
} hash_t;

/** @brief Create a hash over a text.
 *
 *  A hash sum over a given text is created, using the given
 *  algorithm. Space is allocated as needed.
 *
 *  @param algo The algorithm, like md5 or sha1
 *  @param text Plain text used to calculate the hash sum
 *  @return Hash value in a structure
 */
hash_t ggz_hash_create(const char *algo, const char *text);

/** @brief Create a HMAC hash over a text.
 *
 *  Creates a hash sum using a secret key.
 *  Space is allocated as needed and must be freed afterwards.
 *
 *  @param algo The algorithm to use, like md5 or sha1
 *  @param text Plain text used to calculate the hash sum
 *  @param secret Secret key to be used for the HMAC creation
 *  @return Hash value in a structure
 */
hash_t ggz_hmac_create(const char *algo, const char *text, const char *secret);

/** @brief Encodes text to base16.
 *
 *  Plain text with possibly unsafe characters is converted
 *  to the base16 (hex) format through this function.
 *  The returned string is allocated internally and must be freed.
 *
 *  @param text Plain text to encode
 *  @param length Length of the text (which may contain binary characters), in bytes
 *  @return Base16 representation of the text
 */
char *ggz_base16_encode(const char *text, int length);

/** @brief Encodes text to base64.
 *
 *  Plain text with possibly unsafe characters is converted
 *  to the base64 format through this function.
 *  The returned string is allocated internally and must be freed.
 *
 *  @param text Plain text to encode
 *  @param length Length of the text (which may contain binary characters), in bytes
 *  @return Base64 representation of the text
 */
char *ggz_base64_encode(const char *text, int length);

/** @brief Decodes text from base64.
 *
 *  This is the reverse function to ggz_base64_encode().
 *  It will also allocate space as needed.
 *
 *  @param text Text in base64 format
 *  @param length Length of the text, in bytes
 *  @return Native representation, may contain binary characters
 */
char *ggz_base64_decode(const char *text, int length);

/** @brief TLS operation mode.
 *
 *  Hints whether the TLS handshake will happen in either
 *  client or server mode.
 *
 *  @see ggz_tls_enable_fd
 */
typedef enum {
	GGZ_TLS_CLIENT,		/**< Operate as client */
	GGZ_TLS_SERVER		/**< Operate as server */
} GGZTLSType;

/** @brief TLS verification type.
 *
 *  The authentication (verification) model to be used
 *  for the handshake. None means that no certificate
 *  is validated.
 *
 *  @see ggz_tls_enable_fd
 */
typedef enum {
	GGZ_TLS_VERIFY_NONE,		/**< Don't perform verification */
	GGZ_TLS_VERIFY_PEER			/**< Perform validation of the server's cert */
} GGZTLSVerificationType;

/** @brief Initialize TLS support on the server side.
 *
 *  This function ought only be used on the server side.
 *  It sets up the necessary initialization values.
 *
 *  @param certfile File containing the certificate, or NULL
 *  @param keyfile File containing the private key, or NULL
 *  @param password Password to the private key, or NULL
 */
void ggz_tls_init(const char *certfile, const char *keyfile, const char *password);

/** @brief Check TLS support.
 *
 *  Checks if real TLS support is available or communication
 *  will fall back to unencrypted connections.
 *  Even in the case of support, individual connections might
 *  still be unencrypted if the handshake fails.
 *
 *  @return 1 if TLS is supported, 0 if no support is present
 *  @see ggz_tls_enable_fd
 */
int ggz_tls_support_query(void);

/** @brief Name of the TLS implementation.
 *
 *  Returns the name of the TLS layer implementation used
 *  to encrypt connections.
 *
 *  @return TLS implementation name, or NULL if no TLS support is present
 *  @see ggz_tls_support_query
 */
const char *ggz_tls_support_name(void);

/** @brief Enable TLS for a file descriptor.
 *
 *  A TLS handshake is performed for an existing connection on the given
 *  file descriptor. On success, all consecutive data will be encrypted.
 *
 *  @param fdes File descriptor in question
 *  @param whoami Operation mode (client or server)
 *  @param verify Verification mode
 *  @return 1 on success, 0 on failure
 */
int ggz_tls_enable_fd(int fdes, GGZTLSType whoami, GGZTLSVerificationType verify);

/** @brief Disable TLS for a file descriptor.
 *
 *  An existing TLS connection is reset to a normal connection on which
 *  all communication happens without encryption.
 *
 *  @param fdes File descriptor in question
 *  @return 1 on success, 0 on failure
 */
int ggz_tls_disable_fd(int fdes);

/** @brief Write some bytes to a secured file descriptor.
 *
 *  This function acts as a TLS-aware wrapper for write(2).
 *
 *  @param fd File descriptor to use
 *  @param ptr Pointer to the data to write
 *  @param n Length of the data to write, in bytes
 *  @return Actual number of bytes written
 */
size_t ggz_tls_write(int fd, void *ptr, size_t n);

/** @brief Read from a secured file descriptor.
 *
 *  This function acts as a TLS-aware wrapper for read(2).
 *
 *  @param fd File descriptor to use
 *  @param ptr Pointer to a buffer to store the data into
 *  @param n Number of bytes to read, and minimum size of the buffer
 *  @return Actually read number of bytes
 */
size_t ggz_tls_read(int fd, void *ptr, size_t n);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __GGZ_H__ */
