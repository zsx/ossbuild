
package ossbuild.extract;

/**
 * Adapter for {@link IResourceProgressListener}.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public abstract class ResourceProgressListenerAdapter implements IResourceProgressListener {

	@Override
	public void begin(int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long startTime) {
	}

	@Override
	public void reportMessage(IResourceProcessor resource, IResourcePackage pkg, String key, String message) {
	}

	@Override
	public void reportResourceComplete(IResourceProcessor resource, IResourcePackage pkg, int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long duration, String message) {
	}

	@Override
	public void reportPackageComplete(IResourcePackage pkg, int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long duration, String message) {
	}

	@Override
	public void error(Throwable exception, String message) {
	}

	@Override
	public void end(boolean success, int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long endTime) {
	}

}
