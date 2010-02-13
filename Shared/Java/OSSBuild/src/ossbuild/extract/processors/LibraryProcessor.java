
package ossbuild.extract.processors;

import org.w3c.dom.Node;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.ResourceProcessor;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "Library",
	supportsSize = true
)
public class LibraryProcessor extends DefaultResourceProcessor {

	@Override
	protected long requestSize() {
		return super.requestSize();
	}
	
	@Override
	protected boolean loadSettings(final Node node) {
		return true;
	}

	@Override
	protected boolean processResource() {
		return true;
	}

}
