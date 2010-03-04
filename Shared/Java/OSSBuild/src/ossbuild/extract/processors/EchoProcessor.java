
package ossbuild.extract.processors;

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
 * Deletes every file/subdirectory from the provided directory.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "Echo",
	supportsSize = false
)
public class EchoProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_MESSAGE     = "msg"
		, ATTRIBUTE_KEY         = "key"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String msg = StringUtil.empty;
	private String key = StringUtil.empty;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public EchoProcessor() {
	}

	public EchoProcessor(String message) {
		this(StringUtil.empty, message, StringUtil.empty, StringUtil.empty);
	}

	public EchoProcessor(String message, String Title, String Description) {
		this(StringUtil.empty, message, Title, Description);
	}

	public EchoProcessor(String i18nKey, String message, String Title, String Description) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, Title, Description);

		this.msg = message;
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node, final IVariableProcessor varproc) throws XPathException {
		this.msg = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_MESSAGE);
		this.key = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_KEY);
		
		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		progress.reportMessage(this, pkg, StringUtil.isNullOrEmpty(key) ? StringUtil.empty : key, !StringUtil.isNullOrEmpty(msg) ? msg : StringUtil.empty);
		return true;
	}
}
