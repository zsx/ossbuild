
package ossbuild;

/**
 * Utilities for accessing various system attributes and configuration.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public final class Sys {
	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static OS getOS() {
		return OS.getSystemOS();
	}

	public static Arch getArch() {
		return Arch.getSystemArch();
	}

	public static String getPlatformName() {
		return getOS().getPlatformPartName() + "." + getArch().getPlatformPartName();
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static boolean isOS(final OS OS) {
		return (OS.getSystemOS() == OS);
	}

	public static boolean isArch(final Arch Arch) {
		return (Arch.getSystemArch() == Arch);
	}

	public static String createPackageResourcePrefix(final String PackagePrefix) {
		final String prefix = PackagePrefix.trim();
		if (StringUtil.isNullOrEmpty(prefix))
			return "/";
		return '/' + prefix.replace('.', '/') + '/';
	}

	public static String createPlatformPackageName(final String PackagePrefix) {
		final String prefix = PackagePrefix.trim();
		if (StringUtil.isNullOrEmpty(prefix))
			return getPlatformName();
		if (prefix.endsWith("."))
			return prefix + getPlatformName();
		else
			return prefix + "." + getPlatformName();
	}

	public static String createPlatformPackageResourcePrefix(final String PackagePrefix) {
		return createPackageResourcePrefix(createPlatformPackageName(PackagePrefix));
	}

	public static String createPlatformPackageResourceName(final String PackagePrefix, final String ResourceName) {
		return createPlatformPackageResourcePrefix(PackagePrefix) + ResourceName;
	}
	//</editor-fold>
}
