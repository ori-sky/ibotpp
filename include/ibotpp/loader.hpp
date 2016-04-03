#pragma once
#ifndef IBOTPP_LOADER_HPP
#define IBOTPP_LOADER_HPP

#include <string>
#include <boost/container/vector.hpp>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

namespace ibotpp {
	class loader {
	private:
		llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_ids;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_options;
		clang::DiagnosticsEngine diag_engine;
		llvm::IntrusiveRefCntPtr<clang::CompilerInvocation> invocation;
		clang::CompilerInstance compiler;

	public:
		loader(void)
				: diag_ids(new clang::DiagnosticIDs()),
				  diag_options(new clang::DiagnosticOptions()),
				  diag_engine(diag_ids, diag_options.get(),
					new clang::TextDiagnosticPrinter(llvm::errs(), diag_options.get())),
				  invocation(new clang::CompilerInvocation()) {
			compiler.createDiagnostics();
			if(!compiler.hasDiagnostics()) {
				throw std::runtime_error("compiler instance does not have diagnostics");
			}
		}

		std::unique_ptr<llvm::Module> load(const std::string &path) {
			return load_llvm_module(path);
		}

		std::unique_ptr<llvm::Module> load_llvm_module(const std::string &path) {
			boost::container::vector<const char *> args;
			args.emplace_back("-std=c++11");
			args.emplace_back("-include");
			args.emplace_back("module.hpp");
			args.emplace_back(path.c_str());
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
