
package ossbuild.extract;

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

	int getTotalResourceCount();
	long getTotalSize();
	IResourceProcessor[] getResourceProcessors();

	void visit(IVisitor visitor);
}
