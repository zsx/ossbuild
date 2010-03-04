
package ossbuild.platform;

import com.sun.jna.Library;
import com.sun.jna.win32.StdCallLibrary;
import com.sun.jna.win32.W32APIFunctionMapper;
import com.sun.jna.win32.W32APITypeMapper;
import java.util.HashMap;
import java.util.Map;

/**
 * Provides some common functionality for Win32 libraries. The options handle 
 * mapping function names to platform-specific unicode and ascii equivalents.
 * 
 * Courtesy JNA project:
 *     https://jna.dev.java.net/source/browse/jna/trunk/jnalib/src/com/sun/jna/examples/win32/W32API.java?rev=963&view=markup
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@SuppressWarnings("unchecked")
public interface Win32Library extends StdCallLibrary, Win32Errors {
	//<editor-fold defaultstate="collapsed" desc="Options">
	/** Standard options to use the unicode version of a w32 API. */
	public static final Map UNICODE_OPTIONS = new HashMap() {{
			put(Library.OPTION_TYPE_MAPPER, W32APITypeMapper.UNICODE);
			put(Library.OPTION_FUNCTION_MAPPER, W32APIFunctionMapper.UNICODE);
	}};

	/** Standard options to use the ASCII/MBCS version of a w32 API. */
	public static final Map ASCII_OPTIONS = new HashMap() {{
			put(Library.OPTION_TYPE_MAPPER, W32APITypeMapper.ASCII);
			put(Library.OPTION_FUNCTION_MAPPER, W32APIFunctionMapper.ASCII);
	}};
	
	public static final Map DEFAULT_OPTIONS = Boolean.getBoolean("w32.ascii") ? ASCII_OPTIONS : UNICODE_OPTIONS;
	//</editor-fold>
}
