using System;

namespace OSSBuild.WiX {
	public class Namespace {
		#region Constants
		public const string
			  Default   = @"http://schemas.microsoft.com/wix/2006/wi"
			, NetFX     = @"http://schemas.microsoft.com/wix/NetFxExtension"
			, Util      = @"http://schemas.microsoft.com/wix/UtilExtension"
			, IIS		= @"http://schemas.microsoft.com/wix/IIsExtension"
		;
		#endregion

		public const string
			OSSBuild    = @"http://code.google.com/p/ossbuild/wix/2010/ext"
		;
	}
}
