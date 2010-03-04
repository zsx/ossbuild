
package ossbuild.platform;

/**
 * Standard Win32 error values for use with GetLastError().
 * 
 * Courtesy JNA project:
 *     https://jna.dev.java.net/source/browse/jna/trunk/jnalib/src/com/sun/jna/examples/win32/W32Errors.java?rev=7&view=markup
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface Win32Errors {
	public static final int
		  NO_ERROR                  = 0
		, ERROR_INVALID_FUNCTION    = 1
		, ERROR_FILE_NOT_FOUND      = 2
		, ERROR_PATH_NOT_FOUND      = 3
	;
}
