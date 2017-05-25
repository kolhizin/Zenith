#include "vDevice.h"

void zenith::vulkan::vDeviceImpl_::fillQueueDistribution_(const vDeviceQueueReqs & req, const std::vector<vQueueFamilyProperties> &props, std::vector<std::vector<zenith::util::nameid>> &qDistrib) const
{
	VkQueueFlags qFlg = 0;
	if (req.fCompute)
		qFlg |= VK_QUEUE_COMPUTE_BIT;
	if (req.fGraphics)
		qFlg |= VK_QUEUE_GRAPHICS_BIT;
	if (req.fTransfer)
		qFlg |= VK_QUEUE_TRANSFER_BIT;
	if (req.fSparse)
		qFlg |= VK_QUEUE_SPARSE_BINDING_BIT;

	
	size_t ind = 0;
	for(; ind < props.size(); ind++)
		if (((props[ind].generalProperties.queueFlags & qFlg) == qFlg) && (qDistrib[ind].size() < props[ind].generalProperties.queueCount))
		{
			if (req.fPresent)
				if (!props[ind].supportedSurfaces.exist(req.surfaceUID))
					continue;

			qDistrib[ind].push_back(req.uid);
			break;
		}

	if(ind == props.size())
		throw zenith::vulkan::vDeviceException("vDeviceImpl_::vDeviceImpl_: no free device-queues left for required queues!");
}

void zenith::vulkan::vDeviceImpl_::fillFeatureInfo_(VkPhysicalDeviceFeatures & df)
{
	VkPhysicalDeviceFeatures emptyFeatures = {};
	df = emptyFeatures;	
}

void zenith::vulkan::vDeviceImpl_::fillExtsAndLayers_()
{
	std::vector<std::string> usedExts;
	usedExts.reserve(req_.requiredExtensions.size());

	rawLayers_.clear();
	rawLayers_.resize(req_.enabledLayers.size());

	rawExts_.clear();
	rawExts_.resize(req_.requiredExtensions.size());
	for (size_t i = 0; i < req_.requiredExtensions.size(); i++)
	{
		rawExts_[i] = req_.requiredExtensions[i].c_str();
		usedExts.push_back(req_.requiredExtensions[i]);
	}

	for(size_t i = 0; i < req_.enabledLayers.size(); i++)
		for (size_t j = 0; j < req_.enabledLayers[i].requiredExtensions.size(); j++)
			if (std::find(usedExts.begin(), usedExts.end(), req_.enabledLayers[i].requiredExtensions[j]) == usedExts.end())
			{
				usedExts.push_back(req_.enabledLayers[i].requiredExtensions[j]);
				rawExts_.push_back(req_.enabledLayers[i].requiredExtensions[j].c_str());
			}
}

zenith::vulkan::vDeviceImpl_::vDeviceImpl_(const vInstanceImpl_ * inst, const vDeviceReqs & req)
{
	if (!inst)
		throw zenith::vulkan::vInstanceException("vDeviceImpl_::vDeviceImpl_: Uninitialized instance passed into device-constructor!");

	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Selecting physical device.");
	req_ = req;
	physDevice_ = inst->selectPhysicalDevice_(req_);

	std::vector<vQueueFamilyProperties> props(inst->getQFProperties_(physDevice_));
	std::vector<std::vector<zenith::util::nameid>> qDistribution(props.size());
	std::vector<std::vector<float>> qPriorities(props.size());
		
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Assigning queues.");
	
	for (size_t i = 0; i < req_.queues.size(); i++)
		fillQueueDistribution_(req_.queues[i], props, qDistribution);

	size_t numFamilies = 0;
	for (size_t i = 0; i < qDistribution.size(); i++)
		if (!qDistribution[i].empty())
		{
			qPriorities[i].resize(qDistribution[i].size(), 1.0f);
			for (size_t j = 0; j < qDistribution[i].size(); j++)
			{
				const zenith::util::nameid &uid = qDistribution[i][j];
				size_t qid = 0;
				for (; qid < req_.queues.size(); qid++)
					if (req_.queues[qid].uid == uid)
						break;
				qPriorities[i][j] = req.queues[qid].priority;
			}
			numFamilies++;
		}
	
	std::vector<VkDeviceQueueCreateInfo> qcis(numFamilies);

	uint32_t ind = 0;
	for (uint32_t i = 0; i < qDistribution.size(); i++)
	{
		if (qDistribution[i].empty())
			continue;

		qcis[ind].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qcis[ind].flags = 0;
		qcis[ind].pNext = nullptr;
		qcis[ind].queueFamilyIndex = i;
		qcis[ind].queueCount = static_cast<uint32_t>(qDistribution[i].size());
		qcis[ind].pQueuePriorities = &qPriorities[i][0];
	}
	
	fillFeatureInfo_(physDeviceFeatures_);
	fillExtsAndLayers_();

	VkDeviceCreateInfo dci;
	dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.pNext = nullptr;
	dci.flags = 0;

	dci.queueCreateInfoCount = static_cast<uint32_t>(qcis.size());
	dci.pQueueCreateInfos = &qcis[0];

	dci.pEnabledFeatures = &physDeviceFeatures_;

	dci.enabledLayerCount = static_cast<uint32_t>(rawLayers_.size());
	dci.ppEnabledLayerNames = (rawLayers_.empty() ? nullptr : &rawLayers_[0]);

	dci.enabledExtensionCount = static_cast<uint32_t>(rawExts_.size());
	dci.ppEnabledExtensionNames = (rawExts_.empty() ? nullptr : &rawExts_[0]);

	VkResult res;
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Creating device.");
	if ((res = vkCreateDevice(physDevice_, &dci, nullptr, &device_)) != VK_SUCCESS)
		throw vDeviceException(res, "vDeviceImpl_::vDeviceImpl_: vkCreateDevice failed!");
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Device created.");
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Getting queues.");

	for (uint32_t i = 0; i < qDistribution.size(); i++)
	{
		if (qDistribution[i].empty())
			continue;
		for (uint32_t j = 0; j < qDistribution[i].size(); j++)
		{
			vQueueData_ q;

			vkGetDeviceQueue(device_, i, j, &q.queue);
			q.qFamily = i;
			q.supportCompute = (props[i].generalProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) > 0;
			q.supportGraphics = (props[i].generalProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0;
			q.supportTransfer = (props[i].generalProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) > 0;
			q.supportSparse = (props[i].generalProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) > 0;
			q.supportPresent = (!props[i].supportedSurfaces.empty());
			
			queues_.insert(qDistribution[i][j], q);

			vDeviceQueueReqs * r = nullptr;
			for (size_t k = 0; k < req_.queues.size(); k++)
				if (req_.queues[k].uid == qDistribution[i][j])
				{
					r = &req_.queues[k];
					break;
				}

			for (size_t k = 0; k < r->aliases.size(); k++)
				qAliases_.insert(r->aliases[k], qDistribution[i][j]);
		}
	}
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_: Queues enumerated.");
}

zenith::vulkan::vDeviceImpl_::~vDeviceImpl_()
{
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_~: Destroying device.");
	swapchains_.clear();
	vkDestroyDevice(device_, nullptr);
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vDeviceImpl_~: Device destroyed.");
}

void zenith::vulkan::vDeviceImpl_::addSwapchain(const util::nameid & name, const vSwapchainConfig & conf, const vSurface & surf, const VkExtent2D & size)
{
	ZLOG_DEBUG("vDeviceImpl_::addSwapchain: Adding new swapchain.");
	vSwapchain sc(this, surf, conf, size);
	swapchains_.insert(name, std::move(sc));
	ZLOG_DEBUG("vDeviceImpl_::addSwapchain: Swapchain added.");
}

void zenith::vulkan::vDeviceImpl_::removeSwapchain(const util::nameid & name)
{
	ZLOG_DEBUG("vDeviceImpl_::removeSwapchain: Removing swapchain.");	
	swapchains_.remove(name);
	ZLOG_DEBUG("vDeviceImpl_::removeSwapchain: Swapchain removed.");
}


void zenith::vulkan::vDeviceImpl_::recreateSwapchain(const util::nameid & name, const VkExtent2D & size)
{
	ZLOG_DEBUG("vDeviceImpl_::recreateSwapchain: Recreating swapchain.");
	swapchains_.at(name).recreate(size);
	ZLOG_DEBUG("vDeviceImpl_::recreateSwapchain: Swapchain recreated.");
}

void zenith::vulkan::vDevice::create(const vInstance &inst, const vDeviceConfig & conf)
{
	for (size_t i = 0; i < conf.deviceReqOrder.size(); i++)
	{
		size_t idx = 0;
		for (; idx < conf.deviceReqs.size(); idx++)
			if (conf.deviceReqs[idx].uid == conf.deviceReqOrder[i])
				break;
		if (idx == conf.deviceReqs.size())
			continue;
		try
		{
			Impl::create(inst.raw(), conf.deviceReqs[idx]);
			return;
		}
		catch (const VulkanException &)
		{
			if (i + 1 >= conf.deviceReqOrder.size())
				throw;
			zenith::util::zLOG::log(zenith::util::LogType::WARNING, "vDevice::create: Failed attempt at creating of device. Retrying another config.");
		}
	}
	throw vDeviceException("vDevice::create: none of tested configs of device were statisfied!");
}