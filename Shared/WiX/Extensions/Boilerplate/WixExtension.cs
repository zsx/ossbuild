using Microsoft.Tools.WindowsInstallerXml;

namespace OSSBuild.WiX {
	public class WiXExtension : WixExtension {
		private WixPreprocessorExtension extension;

		public override PreprocessorExtension PreprocessorExtension {
			get {
				// If we haven't created the preprocessor then do it now
				if (extension == null)
					extension = new WixPreprocessorExtension();

				// Return the preprocessor
				return extension;
			}
		}
	}
}
