
package ossbuild.extract.processors;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Node;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.IResourcePackage;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.ResourceProcessor;
import ossbuild.extract.ResourceUtils;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "File",
	supportsSize = true
)
public class FileProcessor extends DefaultResourceProcessor {

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public FileProcessor() {
	}

	public FileProcessor(boolean IsTransient, String ResourceName, String SubDirectory, String DestName, String Title, String Description) {
		super(IsTransient, ResourceName, SubDirectory, DestName, Title, Description);
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException {
		this.size = ResourceUtils.sizeFromResource(fullResourceName);
		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		return ResourceUtils.extractResource(
			fullResourceName,
			pkg.filePath(subDirectory, destName),
			shouldBeTransient
		);
	}

}
