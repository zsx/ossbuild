
package ossbuild.extract.processors;

import java.io.File;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Node;
import ossbuild.StringUtil;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.IResourcePackage;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.IVariableProcessor;
import ossbuild.extract.ResourceProcessor;

/**
 * Makes a new directory at the provided path.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "MakeDirectory",
	supportsSize = false
)
public class MakeDirectoryProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_PATH	= "path"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String path = StringUtil.empty;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public MakeDirectoryProcessor() {
	}

	public MakeDirectoryProcessor(File path) {
		this(path.getAbsolutePath(), StringUtil.empty, StringUtil.empty);
	}

	public MakeDirectoryProcessor(String path) {
		this(path, StringUtil.empty, StringUtil.empty);
	}

	public MakeDirectoryProcessor(String path, String Title, String Description) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, Title, Description);

		this.path = path;
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node, final IVariableProcessor varproc) throws XPathException {
		this.path = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_PATH);
		
		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		final String dir = StringUtil.isNullOrEmpty(path) || ".".equalsIgnoreCase(path) ? pkg.getDirectory() : path;
		if (StringUtil.isNullOrEmpty(dir))
			return true;

		final File d = new File(dir);
		if (d.exists())
			return true;

		try {
			return d.mkdirs();
		} catch(Throwable t) {
			return false;
		}
	}
}
