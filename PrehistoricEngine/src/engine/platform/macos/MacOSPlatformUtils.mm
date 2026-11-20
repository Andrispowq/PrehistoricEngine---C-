#include "Includes.hpp"
#include "MacOSPlatformUtils.h"

#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <cstring>

namespace Prehistoric
{
	static NSArray<UTType*>* getAllowedTypes(const char* filter)
	{
		NSMutableArray<UTType*>* types = [NSMutableArray array];

		//Filters use the same description\0*.extension\0 format as the Windows dialogues.
		while (filter && *filter)
		{
			filter += std::strlen(filter) + 1;
			if (!*filter)
				break;

			NSString* patterns = [NSString stringWithUTF8String:filter];
			for (NSString* pattern in [patterns componentsSeparatedByString:@";"])
			{
				pattern = [pattern stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
				if ([pattern isEqualToString:@"*"] || [pattern isEqualToString:@"*.*"])
					return @[];

				if ([pattern hasPrefix:@"*."])
				{
					UTType* type = [UTType typeWithFilenameExtension:[pattern substringFromIndex:2]];
					if (type && ![types containsObject:type])
						[types addObject:type];
				}
			}

			filter += std::strlen(filter) + 1;
		}

		return types;
	}

	static void configurePanel(NSSavePanel* panel, const char* filter)
	{
		panel.allowedContentTypes = getAllowedTypes(filter);
		panel.allowsOtherFileTypes = NO;
		panel.directoryURL = [NSURL fileURLWithPath:[[NSFileManager defaultManager] currentDirectoryPath] isDirectory:YES];
	}

	std::optional<std::string> MacOSFileDialogues::OpenFile(const char* filter)
	{
		if (![NSThread isMainThread])
		{
			PR_LOG_ERROR("File dialogues must be opened on the main thread!\n");
			return std::nullopt;
		}

		@autoreleasepool
		{
			NSOpenPanel* panel = [NSOpenPanel openPanel];
			configurePanel(panel, filter);
			panel.canChooseFiles = YES;
			panel.canChooseDirectories = NO;
			panel.allowsMultipleSelection = NO;

			if ([panel runModal] == NSModalResponseOK)
			{
				const char* path = panel.URL.path.UTF8String;
				if (path)
					return std::string(path);
			}

			return std::nullopt;
		}
	}

	std::optional<std::string> MacOSFileDialogues::SaveFile(const char* filter)
	{
		if (![NSThread isMainThread])
		{
			PR_LOG_ERROR("File dialogues must be opened on the main thread!\n");
			return std::nullopt;
		}

		@autoreleasepool
		{
			NSSavePanel* panel = [NSSavePanel savePanel];
			configurePanel(panel, filter);
			panel.canCreateDirectories = YES;
			panel.extensionHidden = NO;

			if ([panel runModal] == NSModalResponseOK)
			{
				const char* path = panel.URL.path.UTF8String;
				if (path)
					return std::string(path);
			}

			return std::nullopt;
		}
	}
};
