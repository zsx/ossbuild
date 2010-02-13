
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

	boolean filter(final IResourcePackage pkg, final IResourceProcessor processor, final String resourceName);
}
