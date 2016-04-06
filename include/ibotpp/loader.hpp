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
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <ibotpp/module.hpp>

namespace ibotpp {
	class loader {
	private:
		std::unique_ptr<llvm::LLVMContext> llvm_context;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_ids;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_options;
		clang::DiagnosticsEngine diag_engine;
		std::unique_ptr<clang::driver::Driver> driver;
		llvm::IntrusiveRefCntPtr<clang::CompilerInvocation> invocation;
		clang::CompilerInstance compiler;
		llvm::llvm_shutdown_obj llvm_destructor;

	public:
		loader(void)
				: llvm_context(new llvm::LLVMContext()),
				  diag_ids(new clang::DiagnosticIDs()),
				  diag_options(new clang::DiagnosticOptions()),
				  diag_engine(diag_ids, diag_options.get(),
					new clang::TextDiagnosticPrinter(llvm::errs(),
					                                 diag_options.get())),
				  invocation(new clang::CompilerInvocation()) {
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			auto error_or_clang_path = llvm::sys::findProgramByName("clang++");
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

		std::unique_ptr<module> load(const std::string &path) {
			return get_module_value(load_llvm_module("modules/" + path + ".cpp"));
		}

		std::unique_ptr<llvm::Module> load_llvm_module(const std::string &path) {
			auto compilation = driver->BuildCompilation({"-c++", "-std=c++11",
			                                             "-DIBOTPP_IS_MODULE",
			                                             "-Imodules",
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
			//llvm::errs() << "clang invocation: ";
			//jobs.Print(llvm::errs(), " ", true);
			//llvm::errs() << "\n";
			compiler.setInvocation(invocation.get());
			clang::EmitLLVMOnlyAction action(llvm_context.get());
			if(!compiler.ExecuteAction(action)) {
				throw std::runtime_error("failed to execute action");
			}
			return action.takeModule();
		}

		std::unique_ptr<module> get_module_value(std::unique_ptr<llvm::Module> m) {
			auto fn = m->getFunction("module");
			if(!fn) {
				throw std::runtime_error("failed to find module function");
			}
			std::string err;
			auto exec_engine = llvm::EngineBuilder(std::move(m))
				.setEngineKind(llvm::EngineKind::Either)
				.setErrorStr(&err)
				.create();
			if(!exec_engine) {
				throw std::runtime_error("failed to create execution engine: " + err);
			}
			exec_engine->finalizeObject();
			auto value = exec_engine->runFunction(fn, {});
			auto ptr = reinterpret_cast<module *>(value.PointerVal);
			return std::unique_ptr<module>(ptr);
		}
	};
}

#endif
