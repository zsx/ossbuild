
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
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "Var",
	supportsSize = true
)
public class VarProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_NAME    = "name"
		, ATTRIBUTE_VALUE   = "value"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String name = StringUtil.empty;
	private String value = StringUtil.empty;
	private IVariableProcessor varproc = null;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public VarProcessor() {
	}

	public VarProcessor(String Name, String Value) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, StringUtil.empty, StringUtil.empty);
		this.name = Name;
		this.value = Value;
	}

	public VarProcessor(String Name, String Value, String Title, String Description) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, Title, Description);
		this.name = Name;
		this.value = Value;
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node, final IVariableProcessor varproc) throws XPathException {
		this.name = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_NAME);
		this.value = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_VALUE);
		this.varproc = varproc;

		if (varproc == null || StringUtil.isNullOrEmpty(name))
			return true;

		if (value != null)
			return varproc.saveVariable(name, value);
		else
			return varproc.removeVariable(name);
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		return true;
	}
}
