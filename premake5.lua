workspace "uM"
	location "VStudio"
	configurations { "Release", "Debug" }
	platforms { "x32", "X64" }
	startproject "uM"
	pic "On"
	systemversion "10.0.17763.0"
	characterset "ASCII"
	
project "uM"
	kind "ConsoleApp"
	targetname "uM"
	language "C++"
	targetdir "Binary"
	files { "src/**" }
	removefiles { ".." }
	vpaths {
		["Headers/*"] = { "**.hpp", "**.h" }
	}
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	
	
	os.mkdir("Binary")

-- Cleanup
if _ACTION == "clean" then
	os.rmdir("Binary");
	os.rmdir("VStudio");
end
