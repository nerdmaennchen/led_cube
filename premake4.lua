--[[
	Premake script to create the makefiles for LED_Cube
--]]

dofile "premake4_arm.lua"


-- LED_Cubeconfiguration.h solution config
solution "LedCube"
	configurations { "Debug", "Release", "Processor1"}
	platforms { "native", "arm" }

	project "led_cube"
		kind "ConsoleApp"
		language "c"

		includedirs {"src/", "src/framework/include/", "src/target/include"}

		-- global compiler options
		flags {
			
		}
		
		-- extra options
		newoption {
		   trigger     = "component",
		   value       = "nr",
		   description = "Choose target component",
		   allowed = {
		      { "app",  "application" },
		      { "vector",  "VectorTableLib" }
		   }
		}

		buildoptions { "-Wall", "-Wextra", "-Werror", "-std=c99", "-O2"}
		linkoptions {
			"-L."
		}

		local targetDir        = "build/".._OPTIONS.platform.."/"
		local targetDirRelease = targetDir.."release/".._OPTIONS["component"].."/"
		local targetDirDebug   = targetDir.."debug/".._OPTIONS["component"].."/"
		local targetName       = "led_cube"
		local targetSuffix     = ".elf"

	
		targetname(targetName..targetSuffix)
		
			
		--[[
			global config
		--]]

		prebuildcommands { '@echo "\\n\\n--- Starting to build: `date` ---\\n\\n"' }
		postbuildcommands { '@echo "\\n\\n--- Finished build ---\\n\\n"' }

		configuration "DEBUG"
			--[[
			--  valid DEBUG SYMBOLS:
			--    DEBUG
			--]]
			defines { "DEBUG" }
			flags { "Symbols"}
			objdir(targetDirDebug.."/obj/")
			buildoptions {"-g3", "-O0"}
			
			
		configuration "RELEASE"
			flags { "Symbols" }
			defines { "RELEASE"
					,"NO_LOG_OUTPUT" }
			objdir(targetDirRelease.."/obj/")
			buildoptions {"-g0", "-O3"}
			
		--[[
			processor location 
		--]]
		
		configuration "vector"
			kind "StaticLib"
			files {
				"src/target/stm32f4/vectorISR.c"
			}
			linkoptions {
			}
			targetname("vector")
			
		configuration "app"
			files {
				"src/**.c"
			}
			excludes {
				"src/target/stm32f4/vectorISR.c"
			}
			linkoptions {
				"-Tsrc/target/stm32f4/stm32.ld",
				"-lvector",
				"-lm",
				"-lc",
				"-Wl,-Map=led_cube.map,--cref"
			}
			targetname("led_cube"..targetSuffix)

		--[[
			ARM specific config	
		--]]
		configuration "arm"
			defines {
						"STM32F4"
					}
			
			buildoptions { "-mthumb", "-mcpu=cortex-m4", "-msoft-float" }
			linkoptions { "-mthumb -mcpu=cortex-m4 -msoft-float" }
		
			linkoptions {
				"-mfix-cortex-m3-ldrd",
				"-nostartfiles",
			}

