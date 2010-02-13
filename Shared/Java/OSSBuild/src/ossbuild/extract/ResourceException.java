
package ossbuild.extract;

/**
 * Exception regarding resource extraction and/or loading.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class ResourceException extends RuntimeException {
    public ResourceException() {
    }

    public ResourceException(String message) {
        super(message);
    }

    public ResourceException(Throwable cause) {
        super(cause);
    }

    public ResourceException(String message, Throwable cause) {
        super(message, cause);
    }
}
