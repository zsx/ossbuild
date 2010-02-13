
package ossbuild.extract;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
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
	String getName();
	
	boolean load(final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException;
	boolean process(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress);
}
