// ###############################################################################
// 
// Visitlab Playground - Visualization & Computer Graphics Toolkit.
// 
// Copyright (c) 2021-2022 Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// 
// ################################################################################

#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>

#include <iostream>

/**
 * The KEEP_IN_MODULE macro marks a function to be kept in the @p Module and exposed to JavaScript.
 * An alternative to Emscripten's @p bind or @p cwrap.
 * @code
 *	// C++
 *	KEEP_IN_MODULE int getValue() {
 *		return 42;
 *	}
 *	// JavaScript
 *	console.log(Module._getValue());
 * @endcode
 */
#ifndef KEEP_IN_MODULE
#define KEEP_IN_MODULE extern "C" __attribute__((used, visibility("default")))
#endif

 /** The forward declaration for the actual entry point. */
extern "C" int __main__(int argc, char* argv[]);

/** The JavaScript asynchronous calls that need to finish before starting the application. */
EM_JS(void, glue_preint, (), {
	var entry = __glue_main_;
	if (entry) {
		// None of the WebGPU properties appear to survive closure, including Emscripten's own `preinitializedWebGPUDevice`.
		// Which from looking at `library_html5` is probably designed to be initialized in script before loading the WASM.
		if (navigator["gpu"]) {
			navigator["gpu"]["requestAdapter"]().then(function(adapter) {
				adapter["requestDevice"]().then(function(device) {
					Module["preinitializedWebGPUDevice"] = device;
					entry();
				});
			}, function() {
				console.error("No WebGPU adapter; not starting.");
			});
		} else {
			console.error("No support for WebGPU; not starting.");
		}
	} else {
		console.error("Entry point not found; unable to start.");
	}
});

/**
 * The helper method that redirects to call __main__().
 * The function is exposed to Emscripten's @p Module.
 */
KEEP_IN_MODULE
void _glue_main_()
{
	__main__(0, nullptr);
}

/**
 * The entry point for the 'web' application.
 *
 * @param argc the count of program arguments in argv
 * @param argv the program arguments
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	glue_preint();
	return 0;
}
