#
#  Be sure to run `pod spec lint --verbose DrawKit.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see http://docs.cocoapods.org/specification.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |s|

  s.name         = "DrawKit"
  s.version      = "1.0.3"
  s.summary      = "Illustration and vector artwork framework for macOS"
  s.description  = "DrawKit is a software framework that enables the macOS developer to rapidly implement vector drawing and illustration features in a custom application. It is comprehensive, powerful and complete, but it is also highly modular so you can make use of only those parts that you need, or go the whole hog and drop it in as a complete vector drawing solution."
  s.homepage = 'https://drawkit.github.io'
  s.documentation_url = 'https://drawkit.github.io'
  s.screenshot  = "https://raw.githubusercontent.com/DrawKit/DrawKit/master/Documentation/drawkit-sample-capabilities.png"
  
  s.license      = { :type => "MPL2", :file => "DKDrawKit.framework/Resources/LICENCE.txt" }

  s.platform     = :osx, "10.7"
  s.source       = { :git => "https://github.com/DrawKit/DrawKit-podspec.git", :tag => "#{ s.version }" }

  s.author   = { 'Graham Miln' => 'graham.miln@miln.eu' } # Maintainer
  s.social_media_url   = "https://twitter.com/grahammiln"

  s.public_header_files = "DKDrawKit.framework/Headers/*.h"
  s.preserve_paths = "DKDrawKit.framework"
  
  s.vendored_frameworks = "DKDrawKit.framework"
  s.resource = "DKDrawKit.framework"
  
  s.resource = "DKDrawKit.framework"
  
  s.frameworks = "Cocoa", "QuartzCore"
  s.requires_arc = false
  s.xcconfig = {
  	"FRAMEWORK_SEARCH_PATHS" => "$\"(PODS_ROOT)/DrawKit/**\"",
  	"LD_RUNPATH_SEARCH_PATHS" => "@loader_path/../Frameworks",
  	"OTHER_CODE_SIGN_FLAGS" => "--deep"
  }

end
