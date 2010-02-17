
package ossbuild.extract.processors;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import ossbuild.StringUtil;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.IResourcePackage;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.ResourceProcessor;
import ossbuild.extract.ResourceUtils;

/**
 * Attempts to read in and load a shared library. It can try multiple
 * system libraries first. If none of those work, then it will attempt
 * to extract and load the specified resource shared library.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "Library",
	supportsSize = true
)
public class LibraryProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Variables">
	private List<String> systemAttempts = new ArrayList<String>(1);
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public LibraryProcessor() {
	}

	public LibraryProcessor(boolean IsTransient, String ResourceName, String SubDirectory, String DestName, String Title, String Description) {
		super(IsTransient, ResourceName, SubDirectory, DestName, Title, Description);
	}
	//</editor-fold>
	
	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	public void addSystemAttempt(String value) {
		systemAttempts.add(value);
	}

	public void clearSystemAttempts() {
		systemAttempts.clear();
	}
	//</editor-fold>

	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException {
		this.size = ResourceUtils.sizeFromResource(fullResourceName);
		
		NodeList lst;
		Node childNode;

		if ((lst = (NodeList)xpath.evaluate("SystemAttempt", node, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
			return true;

		//Iterate over every <SystemAttempt /> tag, extract its value, and
		//then add it to our list.
		for(int i = 0; i < lst.getLength() && (childNode = lst.item(i)) != null; ++i)
			addSystemAttempt(childNode.getTextContent().trim());
		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		for(String libAttempt : systemAttempts)
			if (!StringUtil.isNullOrEmpty(libAttempt) && ResourceUtils.attemptSystemLibraryLoad(libAttempt))
				return true;

		final File dest = pkg.filePath(subDirectory, destName);
		if (ResourceUtils.extractResource(fullResourceName, dest, shouldBeTransient))
			return ResourceUtils.attemptLibraryLoad(dest.getAbsolutePath());
		return false;
	}
}
