#pragma once
#ifndef IBOTPP_LOADER_HPP
#define IBOTPP_LOADER_HPP

#include <string>
#include <boost/container/vector.hpp>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

namespace ibotpp {
	class loader {
	private:
		llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_ids;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_options;
		clang::DiagnosticsEngine diag_engine;
		std::unique_ptr<clang::driver::Driver> driver;
		llvm::IntrusiveRefCntPtr<clang::CompilerInvocation> invocation;
		clang::CompilerInstance compiler;

	public:
		loader(void)
				: diag_ids(new clang::DiagnosticIDs()),
				  diag_options(new clang::DiagnosticOptions()),
				  diag_engine(diag_ids, diag_options.get(),
					new clang::TextDiagnosticPrinter(llvm::errs(),
					                                 diag_options.get())),
				  invocation(new clang::CompilerInvocation()) {
			auto error_or_clang_path = llvm::sys::findProgramByName("clang");
			if(!error_or_clang_path) {
				throw std::runtime_error("failed to find path of clang executable");
			}
			driver = std::unique_ptr<clang::driver::Driver>(
				new clang::driver::Driver(error_or_clang_path.get(),
			                              llvm::sys::getDefaultTargetTriple(),
			                              diag_engine));
			driver->setCheckInputsExist(false);
			compiler.createDiagnostics();
			if(!compiler.hasDiagnostics()) {
				throw std::runtime_error("failed to create compiler instance with diagnostics");
			}
		}

		std::unique_ptr<llvm::Module> load(const std::string &path) {
			return load_llvm_module(path);
		}

		std::unique_ptr<llvm::Module> load_llvm_module(const std::string &path) {
			auto compilation = driver->BuildCompilation({"-std=c++11",
			                                             "-fsanitize=address",
			                                             "-include", "module.hpp",
			                                             path.c_str()});
			if(!compilation) {
				throw std::runtime_error("failed to build driver compilation");
			}
			auto &jobs = compilation->getJobs();
			auto &cmd = llvm::cast<clang::driver::Command>(*jobs.begin());
			auto &args = cmd.getArguments();
			clang::CompilerInvocation::CreateFromArgs(*invocation, args.data(),
			                                          args.data() + args.size(),
			                                          diag_engine);
			compiler.setInvocation(invocation.get());
			clang::EmitLLVMOnlyAction action;
			if(!compiler.ExecuteAction(action)) {
				throw std::runtime_error("failed to execute action");
			}
			return action.takeModule();
		}
	};
}

#endif
