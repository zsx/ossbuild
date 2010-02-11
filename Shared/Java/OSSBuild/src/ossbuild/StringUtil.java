
package ossbuild;

/**
 * Simple string utility class.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class StringUtil {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		empty = ""
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	public static final boolean isNullOrEmpty(String value) {
		return (value == null || empty.equals(value));
	}
	//</editor-fold>
}
