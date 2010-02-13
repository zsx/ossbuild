
package ossbuild.extract;

import org.w3c.dom.Node;

/**
 * Classes that implement this interface are responsible for being able to
 * load themselves through the {@link #load(org.w3c.dom.Node)} method and
 * then perform extraction or processing in the {@link #process()} method.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final IResourceProcessor
		NONE = null
	;
	
	public static final IResourceProcessor[]
		EMPTY = new IResourceProcessor[0]
	;
	//</editor-fold>

	boolean supportsSize();
	long getSize();
	
	boolean load(Node node);
	boolean process();
}
