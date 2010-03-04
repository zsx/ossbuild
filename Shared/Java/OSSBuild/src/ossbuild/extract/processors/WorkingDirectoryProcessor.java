
package ossbuild.extract.processors;

import java.io.File;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Node;
import ossbuild.StringUtil;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.IResourcePackage;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.ResourceProcessor;

/**
 * Changes the process' working directory (cwd).
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "WorkingDirectory",
	supportsSize = false
)
public class WorkingDirectoryProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_PATH	= "path"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String path = StringUtil.empty;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public WorkingDirectoryProcessor() {
	}

	public WorkingDirectoryProcessor(File path) {
		this(path.getAbsolutePath(), StringUtil.empty, StringUtil.empty);
	}

	public WorkingDirectoryProcessor(String path) {
		this(path, StringUtil.empty, StringUtil.empty);
	}

	public WorkingDirectoryProcessor(String path, String Title, String Description) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, Title, Description);

		this.path = path;
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node) throws XPathException {
		this.path = stringAttributeValue(StringUtil.empty, node, ATTRIBUTE_PATH);
		
		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		final String dir = StringUtil.isNullOrEmpty(path) || ".".equalsIgnoreCase(path) ? ossbuild.Process.getWorkingDirectory() : path;
		if (StringUtil.isNullOrEmpty(dir))
			return true;

		final File d = new File(dir);
		if (!d.exists())
			return false;

		try {
			return ossbuild.Process.setWorkingDirectory(d);
		} catch(Throwable t) {
			return false;
		}
	}
}
