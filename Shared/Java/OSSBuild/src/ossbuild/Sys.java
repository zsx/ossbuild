
package ossbuild;

import ossbuild.init.Loader;

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

	public static boolean isResourceAvailable(final String ResourceName) {
		final String res = (ResourceName.startsWith("/") && ResourceName.length() > 1 ? ResourceName.substring(1) : ResourceName).trim();
		if (StringUtil.isNullOrEmpty(res))
			return false;
		return (Thread.currentThread().getContextClassLoader().getResource(res) != null);
	}

	public static String createPackageResourcePrefix(final String PackagePrefix, final String PackageSuffix) {
		final String prefix = PackagePrefix.trim();
		final String suffix = PackageSuffix.trim();
		return
			'/' +
			(!StringUtil.isNullOrEmpty(prefix) ? prefix.replace('.', '/') + '/' : StringUtil.empty) +
			(!StringUtil.isNullOrEmpty(suffix) ? suffix.replace('.', '/') + '/' : StringUtil.empty)
		;
	}

	public static String createPackageResourcePrefix(final String PackagePrefix) {
		return createPackageResourcePrefix(PackagePrefix, StringUtil.empty);
	}

	public static String createPlatformPackageName(final String PackagePrefix, final String PackageSuffix) {
		final String prefix = PackagePrefix.trim();
		final String suffix = PackageSuffix.trim();

		String ret = getPlatformName();
		
		if (!StringUtil.isNullOrEmpty(prefix)) {
			if (prefix.endsWith("."))
				ret = prefix + ret;
			else
				ret = prefix + "." + ret;
		}

		if (!StringUtil.isNullOrEmpty(suffix)) {
			if (suffix.startsWith("."))
				ret = ret + suffix;
			else
				ret = ret + "." + suffix;
		}

		return ret;
	}

	public static String createPlatformPackageName(final String PackagePrefix) {
		return createPlatformPackageName(PackagePrefix, StringUtil.empty);
	}

	public static String createPlatformPackageResourcePrefix(final String PackagePrefix, final String PackageSuffix) {
		return createPackageResourcePrefix(createPlatformPackageName(PackagePrefix, PackageSuffix));
	}

	public static String createPlatformPackageResourcePrefix(final String PackagePrefix) {
		return createPackageResourcePrefix(createPlatformPackageName(PackagePrefix));
	}

	public static String createPlatformPackageResourceName(final String PackagePrefix, final String PackageSuffix, final String ResourceName) {
		return createPlatformPackageResourcePrefix(PackagePrefix, PackageSuffix) + ResourceName;
	}

	public static String createPlatformPackageResourceName(final String PackagePrefix, final String ResourceName) {
		return createPlatformPackageResourcePrefix(PackagePrefix) + ResourceName;
	}

	public static boolean initialize() {
		try {
			return Loader.initialize();
		} catch(Throwable t) {
			throw new RuntimeException(t);
		}
	}
	//</editor-fold>
}
