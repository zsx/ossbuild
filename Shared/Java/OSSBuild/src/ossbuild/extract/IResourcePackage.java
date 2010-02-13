
package ossbuild.extract;

import java.io.File;

/**
 * Interface that describes a series of resources for extraction.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IResourcePackage extends Iterable<IResourceProcessor> {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final IResourcePackage
		NONE = null
	;

	public static final IResourcePackage[]
		EMPTY = new IResourcePackage[0]
	;
	//</editor-fold>
	
	//<editor-fold defaultstate="collapsed" desc="IVisitor">
	public static interface IVisitor {
		boolean each(final IResourcePackage Package, final IResourceProcessor Processor);
	}
	//</editor-fold>

	String getName();
	long getTotalSize();
	String getDirectory();
	int getTotalResourceCount();
	IResourceProcessor[] getResourceProcessors();

	String resourcePath(final String ResourceName);
	File filePath(final String SubDirectory, final String FileName);
	File filePath(final String FileName);
	void calculateTotals();

	void visit(IVisitor visitor);
}
