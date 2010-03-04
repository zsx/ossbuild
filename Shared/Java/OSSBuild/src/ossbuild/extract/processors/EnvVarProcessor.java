
package ossbuild.extract.processors;

import java.io.File;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathException;
import org.w3c.dom.Node;
import ossbuild.Path;
import ossbuild.StringUtil;
import ossbuild.Sys;
import ossbuild.extract.DefaultResourceProcessor;
import ossbuild.extract.IResourcePackage;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.IVariableProcessor;
import ossbuild.extract.ResourceProcessor;

/**
 * Sets or unsets a local process' environment variables.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@ResourceProcessor(
	tagName = "EnvVar",
	supportsSize = false
)
public class EnvVarProcessor extends DefaultResourceProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  ATTRIBUTE_NAME		= "name"
		, ATTRIBUTE_VALUE		= "value"
		, ATTRIBUTE_UNSET		= "unset"
		, ATTRIBUTE_ASFILE		= "asFile"
		, ATTRIBUTE_PREPEND		= "prepend"
		, ATTRIBUTE_APPEND		= "append"
		, ATTRIBUTE_SEPARATOR	= "separator"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private String name = StringUtil.empty;
	private String value = StringUtil.empty;
	private boolean unset = false;
	private boolean asFile = false;
	private boolean prepend = false;
	private boolean append = false;
	private String separator = StringUtil.empty;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public EnvVarProcessor() {
	}

	public EnvVarProcessor(String Name, String Value, boolean AsFile, boolean Prepend, boolean Append, boolean Unset, String Separator, String Title, String Description) {
		super(false, StringUtil.empty, StringUtil.empty, StringUtil.empty, Title, Description);

		this.name = Name;
		this.value = Value;
		this.unset = Unset;
		this.asFile = AsFile;
		this.prepend = Prepend;
		this.append = Append;
		this.separator = Separator;
	}
	//</editor-fold>
	
	@Override
	protected boolean loadSettings(final String fullResourceName, final IResourcePackage pkg, final XPath xpath, final Node node, final IVariableProcessor varproc) throws XPathException {
		this.name = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_NAME);
		this.value = stringAttributeValue(varproc, StringUtil.empty, node, ATTRIBUTE_VALUE);
		this.separator = stringAttributeValue(varproc, Path.pathSeparator, node, ATTRIBUTE_SEPARATOR);
		
		this.unset = boolAttributeValue(varproc, false, node, ATTRIBUTE_UNSET);
		this.asFile = boolAttributeValue(varproc, false, node, ATTRIBUTE_ASFILE);
		this.prepend = boolAttributeValue(varproc, false, node, ATTRIBUTE_PREPEND);
		this.append = boolAttributeValue(varproc, false, node, ATTRIBUTE_APPEND);

		return true;
	}

	@Override
	protected boolean processResource(final String fullResourceName, final IResourcePackage pkg, final IResourceProgressListener progress) {
		if (StringUtil.isNullOrEmpty(name))
			return true;

		final String existingValue = Sys.getEnvironmentVariable(name);

		String preppedValue;
		if (!asFile)
			preppedValue = value;
		else
			preppedValue = new File(value).getAbsolutePath();

		String destValue;
		if (prepend && append)
			destValue = preppedValue + separator + existingValue + separator + preppedValue;
		else if (prepend && !append)
			destValue = preppedValue + separator + existingValue;
		else if (!prepend && append)
			destValue = existingValue + separator + preppedValue;
		else
			destValue = preppedValue;

		if (!unset)
			return Sys.setEnvironmentVariable(name, destValue);
		else
			return Sys.unsetEnvironmentVariable(name);
	}
}
