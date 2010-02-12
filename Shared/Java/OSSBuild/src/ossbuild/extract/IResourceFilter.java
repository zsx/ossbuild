
package ossbuild.extract;

/**
 * Accepts or rejects a library or file being loaded/extracted.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IResourceFilter {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final IResourceFilter None = null;
	//</editor-fold>

	boolean filterForExtraction(String name);
	boolean filterForLibraryLoad(String name);
}
